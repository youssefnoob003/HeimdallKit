#include "heim/common.h"
#include "heim/io.h"
#include "heim/io_internal.h"
#include "heim/signal_internal.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RAW_BUFFER_SAMPLES 512

HeimStatus heim_sig_io_write_raw(const HeimSig *sig, const char *filepath) {
  HEIM_CHECK_NOT_NULL(filepath, sig);
  if (!heim_sig_is_valid(sig))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");

  HeimOutputFile output;
  if (!heim_io_open_output(filepath, &output))
    return HEIM_MAKE_ERR(HEIM_IO_ERROR);
  FILE *fp = output.fp;

  double buffer[RAW_BUFFER_SAMPLES * 2];
  size_t buf_idx = 0;
  bool ok = true;

  for (size_t i = 0; i < sig->len && ok; i++) {
    buffer[buf_idx++] = sig->i_data[i];
    buffer[buf_idx++] = sig->q_data[i];
    if (buf_idx == RAW_BUFFER_SAMPLES * 2) {
      ok = (fwrite(buffer, sizeof(double), buf_idx, fp) == buf_idx);
      buf_idx = 0;
    }
  }
  if (buf_idx > 0 && ok) {
    ok = (fwrite(buffer, sizeof(double), buf_idx, fp) == buf_idx);
  }

  return heim_io_close_output(&output, filepath, ok) ? HEIM_OK : HEIM_MAKE_ERR(HEIM_IO_ERROR);
}

HeimStatus heim_sig_io_read_raw(const char *filepath, int64_t n_start, double step,
                                HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, filepath);
  *out_sig = NULL;
  if (!isfinite(step) || step <= 0.0)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "step must be finite and > 0");

  FILE *fp = fopen(filepath, "rb");
  if (!fp)
    return HEIM_MAKE_ERR(HEIM_IO_ERROR);

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return HEIM_MAKE_ERR(HEIM_IO_ERROR);
  }
  long file_size = ftell(fp);
  if (file_size < 0) {
    fclose(fp);
    return HEIM_MAKE_ERR(HEIM_IO_ERROR);
  }
  if (file_size == 0 || (size_t)file_size % (2 * sizeof(double)) != 0) {
    fclose(fp);
    return HEIM_MAKE_ERR(HEIM_FORMAT_ERROR);
  }

  size_t count = (size_t)file_size / (2 * sizeof(double));
  if (count > HEIM_MAX_READ_SAMPLES) {
    fclose(fp);
    return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "sample count exceeds HEIM_MAX_READ_SAMPLES");
  }

  rewind(fp);

  HeimStatus status = heim_sig_alloc(n_start, step, count, out_sig);
  if (HEIM_IS_ERR(status)) {
    fclose(fp);
    return status;
  }

  double buffer[RAW_BUFFER_SAMPLES * 2];
  size_t read_count = 0;
  while (read_count < count) {
    size_t to_read = HEIM_MIN((size_t)RAW_BUFFER_SAMPLES, count - read_count);
    if (fread(buffer, sizeof(double) * 2, to_read, fp) != to_read) {
      heim_sig_free(*out_sig);
      *out_sig = NULL;
      fclose(fp);
      return HEIM_MAKE_ERR(HEIM_FORMAT_ERROR);
    }
    for (size_t i = 0; i < to_read; i++) {
      (*out_sig)->i_data[read_count + i] = buffer[2 * i];
      (*out_sig)->q_data[read_count + i] = buffer[2 * i + 1];
    }
    read_count += to_read;
  }

  if (fclose(fp) != 0) {
    heim_sig_free(*out_sig);
    *out_sig = NULL;
    return HEIM_MAKE_ERR(HEIM_IO_ERROR);
  }

  return HEIM_OK;
}
