#ifndef HEIM_INTERNAL_SIGNAL_H
#define HEIM_INTERNAL_SIGNAL_H

#include "heim/signal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct HeimSig {
  double *i_data;
  double *q_data;
  size_t len;
  int64_t n_start;
  double step;
};

/**
 * @brief Checks if a signal index range [n_start, n_start + len - 1] fits inside signed 64-bit
 * bounds.
 *
 * @param n_start Starting sample index.
 * @param len Number of samples in range.
 * @return True if range is valid and fits within int64_t without overflowing, false otherwise.
 */
static inline bool heim_sig_index_range_fits(int64_t n_start, size_t len) {
  if (len == 0 || (uint64_t)len > (uint64_t)INT64_MAX)
    return false;

  return n_start <= INT64_MAX - (int64_t)(len - 1);
}

/**
 * @brief Computes ending index (n_start + len - 1) directly without validation checks.
 *
 * Precondition: sig is non-NULL and heim_sig_is_valid(sig) is true.
 *
 * @param sig The signal to query.
 * @return Ending discrete sample index.
 */
static inline int64_t heim_sig_end_unchecked(const HeimSig *sig) {
  return sig->n_start + (int64_t)(sig->len - 1);
}

/**
 * @brief Safely computes inclusive range length (last - first + 1) with overflow detection.
 *
 * @param first Starting index.
 * @param last Ending index (must be >= first).
 * @param[out] out_length Pointer to receive computed length.
 * @return True if difference + 1 fits within size_t without overflow, false otherwise.
 */
static inline bool heim_sig_range_length(int64_t first, int64_t last, size_t *out_length) {
  if (!out_length || first > last)
    return false;

  uint64_t difference = (uint64_t)last - (uint64_t)first;
  if (difference == UINT64_MAX || difference + 1 > SIZE_MAX)
    return false;

  *out_length = (size_t)(difference + 1);
  return true;
}

#endif
