#include "heim/common.h"
#include "heim/complex.h"
#include "heim/math_internal.h"
#include "heim/signal.h"
#include "heim/signal_internal.h"

#include <math.h>
#include <stdint.h>

static inline bool complex_almost_equal(HeimComplex a, HeimComplex b) {
  return heim_almost_equal(a.real, b.real) && heim_almost_equal(a.imag, b.imag);
}

bool heim_sig_an_equal_overlap(const HeimSig *sig1, const HeimSig *sig2) {
  if (!heim_sig_compatible(sig1, sig2))
    return false;

  int64_t end1 = heim_sig_end_unchecked(sig1);
  int64_t end2 = heim_sig_end_unchecked(sig2);
  int64_t left = HEIM_MAX(sig1->n_start, sig2->n_start);
  int64_t right = HEIM_MIN(end1, end2);
  if (left > right)
    return true;

  size_t count;
  if (!heim_sig_range_length(left, right, &count))
    return false;

  for (size_t i = 0; i < count; i++) {
    int64_t n = left + (int64_t)i;
    HeimComplex s1 = heim_sig_get_sample_complex(sig1, n);
    HeimComplex s2 = heim_sig_get_sample_complex(sig2, n);
    if (!complex_almost_equal(s1, s2))
      return false;
  }
  return true;
}

bool heim_sig_an_equal(const HeimSig *sig1, const HeimSig *sig2) {
  if (!heim_sig_compatible(sig1, sig2))
    return false;

  int64_t end1 = heim_sig_end_unchecked(sig1);
  int64_t end2 = heim_sig_end_unchecked(sig2);
  int64_t left = HEIM_MIN(sig1->n_start, sig2->n_start);
  int64_t right = HEIM_MAX(end1, end2);

  size_t count;
  if (!heim_sig_range_length(left, right, &count))
    return false;

  for (size_t i = 0; i < count; i++) {
    int64_t n = left + (int64_t)i;
    HeimComplex s1 = heim_sig_get_sample_complex(sig1, n);
    HeimComplex s2 = heim_sig_get_sample_complex(sig2, n);
    if (!complex_almost_equal(s1, s2))
      return false;
  }
  return true;
}

static bool sig_has_parity(const HeimSig *sig, bool odd) {
  if (!heim_sig_is_valid(sig))
    return false;

  for (size_t i = 0; i < sig->len; i++) {
    int64_t n = sig->n_start + (int64_t)i;
    HeimComplex s = heim_complex(sig->i_data[i], sig->q_data[i]);
    HeimComplex rev =
        (n != INT64_MIN) ? heim_sig_get_sample_complex(sig, -n) : (HeimComplex){0.0, 0.0};
    if (odd) {
      if (!heim_almost_equal(s.real + rev.real, 0.0) || !heim_almost_equal(s.imag + rev.imag, 0.0))
        return false;
    } else {
      if (!heim_almost_equal(s.real - rev.real, 0.0) || !heim_almost_equal(s.imag - rev.imag, 0.0))
        return false;
    }
  }
  return true;
}

bool heim_sig_an_is_odd(const HeimSig *sig) {
  return sig_has_parity(sig, true);
}

bool heim_sig_an_is_even(const HeimSig *sig) {
  return sig_has_parity(sig, false);
}

HeimStatus heim_sig_an_energy_range(const HeimSig *sig, int64_t first_n, int64_t last_n,
                                    double *out_energy) {
  HEIM_CHECK_NOT_NULL(out_energy, sig);
  *out_energy = 0.0;
  if (!heim_sig_is_valid(sig) || first_n > last_n)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid signal or first_n > last_n");

  int64_t sig_end = heim_sig_end_unchecked(sig);
  int64_t left = HEIM_MAX(first_n, sig->n_start);
  int64_t right = HEIM_MIN(last_n, sig_end);
  if (left > right)
    return HEIM_OK;

  size_t first = (size_t)((uint64_t)left - (uint64_t)sig->n_start);
  size_t count;
  if (!heim_sig_range_length(left, right, &count))
    return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "range length overflow");

  for (size_t i = 0; i < count; i++) {
    HeimComplex s = heim_complex(sig->i_data[first + i], sig->q_data[first + i]);
    *out_energy += heim_complex_abs_sq(s);
  }
  return HEIM_OK;
}

HeimStatus heim_sig_an_energy(const HeimSig *sig, double *out_energy) {
  HEIM_CHECK_NOT_NULL(out_energy, sig);
  if (!heim_sig_is_valid(sig))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");

  return heim_sig_an_energy_range(sig, sig->n_start, heim_sig_end_unchecked(sig), out_energy);
}

HeimStatus heim_sig_an_power_range(const HeimSig *sig, int64_t first_n, int64_t last_n,
                                   double *out_power) {
  HEIM_CHECK_NOT_NULL(out_power);
  *out_power = 0.0;
  if (first_n > last_n)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "first_n > last_n");

  double energy;
  HEIM_PROPAGATE_ERR(heim_sig_an_energy_range(sig, first_n, last_n, &energy));

  long double count = (long double)((uint64_t)last_n - (uint64_t)first_n) + 1.0L;
  if (!(count > 0.0L))
    return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "sample count overflow");

  *out_power = (double)((long double)energy / count);
  return HEIM_OK;
}

HeimStatus heim_sig_an_power(const HeimSig *sig, int64_t radius, double *out_power) {
  HEIM_CHECK_NOT_NULL(out_power);
  *out_power = 0.0;
  if (radius < 0)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "radius must be >= 0");
  if (radius > INT64_MAX / 2)
    return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "radius * 2 would overflow");

  return heim_sig_an_power_range(sig, -radius, radius, out_power);
}
