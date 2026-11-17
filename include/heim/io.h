#ifndef HEIM_IO_H
#define HEIM_IO_H

#include "heim/common.h"
#include "heim/signal.h"

#include <stddef.h>

HEIM_BEGIN_EXTERN_C

#ifndef HEIM_MAX_READ_SAMPLES
#define HEIM_MAX_READ_SAMPLES ((size_t)10000000)
#endif

/**
 * @brief Writes signal samples to a raw unframed binary file of consecutive {I, Q} double pairs.
 *
 * Samples are written in the host's native byte order. Files produced on one architecture may not
 * be readable on another with different endianness. For portable serialization, use
 * heim_sig_io_write_heim() instead.
 *
 * Uses atomic temporary file write and replacement to prevent file corruption.
 *
 * @param sig The source signal to write.
 * @param filepath Destination filesystem path.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid signal,
 *         or HEIM_IO_ERROR on file write failure.
 */
HeimStatus heim_sig_io_write_raw(const HeimSig *sig, const char *filepath);

/**
 * @brief Reads raw unframed double-precision {I, Q} pairs from a binary file into a newly allocated
 * signal.
 *
 * Expects samples in the host's native byte order. Files produced on a different architecture may
 * be unreadable. For portable deserialization, use heim_sig_io_read_heim() instead.
 *
 * @param filepath Source filesystem path.
 * @param n_start Starting discrete sample index to assign to the signal.
 * @param step Sample spacing metadata.
 * @param[out] out_sig Pointer to receive the read signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid step,
 *         HEIM_IO_ERROR on file access error, HEIM_FORMAT_ERROR on unaligned file size or read
 * truncation, HEIM_OVERFLOW if sample count exceeds HEIM_MAX_READ_SAMPLES, or any error returned by
 * heim_sig_alloc().
 */
HeimStatus heim_sig_io_read_raw(const char *filepath, int64_t n_start, double step,
                                HeimSig **out_sig);

/**
 * @brief Writes a signal to a self-describing `.heim` binary format with metadata header.
 *
 * Stores little-endian magic bytes, n_start, step, length, and IEEE-754 binary64 samples
 * atomically.
 *
 * @param sig The source signal to write.
 * @param filepath Destination filesystem path.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid signal,
 *         or HEIM_IO_ERROR on file write failure.
 */
HeimStatus heim_sig_io_write_heim(const HeimSig *sig, const char *filepath);

/**
 * @brief Reads a signal from a self-describing `.heim` binary file.
 *
 * Parses metadata header (magic, version, n_start, step, length) and sample payload.
 *
 * @param filepath Source filesystem path.
 * @param[out] out_sig Pointer to receive the read signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments, HEIM_IO_ERROR on file access
 * error, HEIM_FORMAT_ERROR on invalid header or corrupted stream, or any error returned by
 * heim_sig_alloc().
 */
HeimStatus heim_sig_io_read_heim(const char *filepath, HeimSig **out_sig);
HEIM_END_EXTERN_C

#endif
