#include "heim/common.h"
#include "heim/io.h"
#include "heim/io_internal.h"
#include "heim/signal_internal.h"

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FORMAT_BUFFER_SAMPLES 512

static const unsigned char HEIM_META_MAGIC[8] = {'H', 'E', 'I', 'M', 'E', 'T', 'A', 1};

_Static_assert(sizeof(double) == 8, "Heimdall files require 64-bit doubles");
_Static_assert(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024,
               "Heimdall files require IEEE-754 binary64 doubles");

static void encode_u32_le(uint32_t val, unsigned char b[4]) {
  for (size_t i = 0; i < 4; i++)
    b[i] = (unsigned char)(val >> (8 * i));
}

static void encode_u64_le(uint64_t val, unsigned char b[8]) {
  for (size_t i = 0; i < 8; i++)
    b[i] = (unsigned char)(val >> (8 * i));
}

static uint32_t decode_u32_le(const unsigned char b[4]) {
  uint32_t val = 0;
  for (size_t i = 0; i < 4; i++)
    val |= (uint32_t)b[i] << (8 * i);
  return val;
}

static uint64_t decode_u64_le(const unsigned char b[8]) {
  uint64_t val = 0;
  for (size_t i = 0; i < 8; i++)
    val |= (uint64_t)b[i] << (8 * i);
  return val;
}

static uint64_t encode_i64_zigzag(int64_t val) {
  return (val >= 0) ? ((uint64_t)val * 2) : ((uint64_t)(-(val + 1)) * 2 + 1);
}

static int64_t decode_i64_zigzag(uint64_t val) {
  int64_t mag = (int64_t)(val >> 1);
  return (val & 1) ? -mag - 1 : mag;
}

static void encode_double_le(double val, unsigned char b[8]) {
  uint64_t bits;
  memcpy(&bits, &val, sizeof(bits));
  encode_u64_le(bits, b);
}

static double decode_double_le(const unsigned char b[8]) {
  uint64_t bits = decode_u64_le(b);
  double val;
  memcpy(&val, &bits, sizeof(val));
  return val;
}

HeimStatus heim_sig_io_write_heim(const HeimSig *sig, const char *filepath) {
  HEIM_CHECK_NOT_NULL(filepath, sig);
  if (!heim_sig_is_valid(sig))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");

  HeimOutputFile output;
  if (!heim_io_open_output(filepath, &output))
    return HEIM_MAKE_ERR(HEIM_IO_ERROR);
  FILE *fp = output.fp;

  /* 40-byte header: magic(8), ver(4), res(4), n_start(8), step(8), len(8) */
  unsigned char header[40];
  memcpy(header, HEIM_META_MAGIC, 8);
  encode_u32_le(1, header + 8);
  encode_u32_le(0, header + 12);
  encode_u64_le(encode_i64_zigzag(sig->n_start), header + 16);
  encode_double_le(sig->step, header + 24);
  encode_u64_le((uint64_t)sig->len, header + 32);

  bool ok = (fwrite(header, 1, sizeof(header), fp) == sizeof(header));

  unsigned char buf[FORMAT_BUFFER_SAMPLES * 16];
  size_t idx = 0;
  for (size_t i = 0; ok && i < sig->len; i++) {
    encode_double_le(sig->i_data[i], buf + idx);
    encode_double_le(sig->q_data[i], buf + idx + 8);
    idx += 16;
    if (idx == sizeof(buf)) {
      ok = (fwrite(buf, 1, idx, fp) == idx);
      idx = 0;
    }
  }
  if (idx > 0 && ok) {
    ok = (fwrite(buf, 1, idx, fp) == idx);
  }

  return heim_io_close_output(&output, filepath, ok) ? HEIM_OK : HEIM_MAKE_ERR(HEIM_IO_ERROR);
}

HeimStatus heim_sig_io_read_heim(const char *filepath, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, filepath);
  *out_sig = NULL;

  FILE *fp = fopen(filepath, "rb");
  if (!fp)
    return HEIM_MAKE_ERR(HEIM_IO_ERROR);

  unsigned char header[40];
  if (fread(header, 1, sizeof(header), fp) != sizeof(header)) {
    fclose(fp);
    return HEIM_MAKE_ERR(HEIM_FORMAT_ERROR);
  }

  if (memcmp(header, HEIM_META_MAGIC, 8) != 0 || decode_u32_le(header + 8) != 1 ||
      decode_u32_le(header + 12) != 0) {
    fclose(fp);
    return HEIM_MAKE_ERR(HEIM_FORMAT_ERROR);
  }

  int64_t n_start = decode_i64_zigzag(decode_u64_le(header + 16));
  double step = decode_double_le(header + 24);
  uint64_t length = decode_u64_le(header + 32);

  if (!isfinite(step) || step <= 0.0 || length == 0 || length > HEIM_MAX_READ_SAMPLES ||
      length > SIZE_MAX) {
    fclose(fp);
    return HEIM_MAKE_ERR(HEIM_FORMAT_ERROR);
  }

  HeimSig *sig = NULL;
  HeimStatus status = heim_sig_alloc(n_start, step, (size_t)length, &sig);
  if (HEIM_IS_ERR(status)) {
    fclose(fp);
    return status;
  }

  unsigned char buf[FORMAT_BUFFER_SAMPLES * 16];
  size_t read_samples = 0;
  bool ok = true;

  while (read_samples < (size_t)length && ok) {
    size_t to_read = HEIM_MIN((size_t)FORMAT_BUFFER_SAMPLES, (size_t)length - read_samples);
    size_t bytes_to_read = to_read * 16;
    if (fread(buf, 1, bytes_to_read, fp) != bytes_to_read) {
      ok = false;
      break;
    }
    for (size_t i = 0; i < to_read; i++) {
      sig->i_data[read_samples + i] = decode_double_le(buf + i * 16);
      sig->q_data[read_samples + i] = decode_double_le(buf + i * 16 + 8);
    }
    read_samples += to_read;
  }

  if (ok) {
    int extra = fgetc(fp);
    if (extra != EOF || ferror(fp))
      ok = false;
  }

  if (fclose(fp) != 0)
    ok = false;

  if (!ok) {
    heim_sig_free(sig);
    return HEIM_MAKE_ERR(HEIM_FORMAT_ERROR);
  }

  *out_sig = sig;
  return HEIM_OK;
}
