#include "heim/math_internal.h"
#include "heim/signal.h"
#include "heim/signal_internal.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

bool heim_sig_is_valid(const HeimSig *sig) {
  return sig && sig->i_data && sig->q_data && heim_sig_index_range_fits(sig->n_start, sig->len) &&
         isfinite(sig->step) && sig->step > 0.0;
}

HeimStatus heim_sig_alloc(int64_t n_start, double step, size_t len, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig);
  *out_sig = NULL;

  if (len == 0)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "len must be > 0");
  if (!isfinite(step) || step <= 0.0)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "step must be finite and > 0");
  if (len > SIZE_MAX / sizeof(double))
    return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "len exceeds memory limit");
  if (!heim_sig_index_range_fits(n_start, len))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "index range overflow");

  HeimSig *sig = malloc(sizeof(*sig));
  if (!sig)
    return HEIM_MAKE_ERR(HEIM_ALLOCATION_FAILED);

  sig->i_data = calloc(len, sizeof(*sig->i_data));
  sig->q_data = calloc(len, sizeof(*sig->q_data));
  if (!sig->i_data || !sig->q_data) {
    free(sig->i_data);
    free(sig->q_data);
    free(sig);
    return HEIM_MAKE_ERR(HEIM_ALLOCATION_FAILED);
  }

  sig->len = len;
  sig->n_start = n_start;
  sig->step = step;
  *out_sig = sig;
  return HEIM_OK;
}

HeimStatus heim_sig_from_arrays(int64_t n_start, double step, size_t len, const double *i_values,
                                const double *q_values, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, i_values, q_values);
  *out_sig = NULL;

  HEIM_PROPAGATE_ERR(heim_sig_alloc(n_start, step, len, out_sig));

  memcpy((*out_sig)->i_data, i_values, len * sizeof(double));
  memcpy((*out_sig)->q_data, q_values, len * sizeof(double));
  return HEIM_OK;
}

HeimStatus heim_sig_copy(const HeimSig *sig, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, sig);
  *out_sig = NULL;
  if (!heim_sig_is_valid(sig))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");

  return heim_sig_from_arrays(sig->n_start, sig->step, sig->len, sig->i_data, sig->q_data, out_sig);
}

void heim_sig_free(HeimSig *sig) {
  if (!sig)
    return;
  free(sig->i_data);
  free(sig->q_data);
  free(sig);
}

size_t heim_sig_len(const HeimSig *sig) {
  return heim_sig_is_valid(sig) ? sig->len : 0;
}

int64_t heim_sig_start(const HeimSig *sig) {
  return heim_sig_is_valid(sig) ? sig->n_start : 0;
}

int64_t heim_sig_end(const HeimSig *sig) {
  return heim_sig_is_valid(sig) ? heim_sig_end_unchecked(sig) : 0;
}

double heim_sig_step(const HeimSig *sig) {
  return heim_sig_is_valid(sig) ? sig->step : 0.0;
}

bool heim_sig_contains(const HeimSig *sig, int64_t n) {
  if (!heim_sig_is_valid(sig) || n < sig->n_start)
    return false;
  return (uint64_t)n - (uint64_t)sig->n_start < (uint64_t)sig->len;
}

HeimStatus heim_sig_get_sample(const HeimSig *sig, int64_t n, double *i, double *q) {
  HEIM_CHECK_NOT_NULL(i, q, sig);
  *i = 0.0;
  *q = 0.0;
  if (!heim_sig_is_valid(sig))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");
  if (!heim_sig_contains(sig, n))
    return HEIM_MAKE_ERR(HEIM_OUT_OF_RANGE);

  size_t index = (size_t)((uint64_t)n - (uint64_t)sig->n_start);
  *i = sig->i_data[index];
  *q = sig->q_data[index];
  return HEIM_OK;
}

HeimComplex heim_sig_get_sample_complex(const HeimSig *sig, int64_t n) {
  if (!heim_sig_is_valid(sig) || !heim_sig_contains(sig, n))
    return (HeimComplex){0.0, 0.0};

  size_t index = (size_t)((uint64_t)n - (uint64_t)sig->n_start);
  return (HeimComplex){sig->i_data[index], sig->q_data[index]};
}

HeimStatus heim_sig_set_sample(HeimSig *sig, int64_t n, double i, double q) {
  HEIM_CHECK_NOT_NULL(sig);
  if (!heim_sig_is_valid(sig))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");
  if (!heim_sig_contains(sig, n))
    return HEIM_MAKE_ERR(HEIM_OUT_OF_RANGE);

  size_t index = (size_t)((uint64_t)n - (uint64_t)sig->n_start);
  sig->i_data[index] = i;
  sig->q_data[index] = q;
  return HEIM_OK;
}

bool heim_sig_compatible(const HeimSig *sig1, const HeimSig *sig2) {
  return heim_sig_is_valid(sig1) && heim_sig_is_valid(sig2) &&
         heim_almost_equal(sig1->step, sig2->step);
}
