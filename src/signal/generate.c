#include "heim/signal.h"
#include "heim/signal_internal.h"

#include <math.h>
#include <stdint.h>

HeimStatus heim_sig_gen_create(int64_t n_start, double step, size_t len, HeimSigMapper mapper,
                               HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, mapper);
  *out_sig = NULL;

  HEIM_PROPAGATE_ERR(heim_sig_alloc(n_start, step, len, out_sig));

  for (size_t i = 0; i < len; i++) {
    int64_t n = n_start + (int64_t)i;
    mapper((double)n * step, &(*out_sig)->i_data[i], &(*out_sig)->q_data[i]);
  }
  return HEIM_OK;
}

HeimStatus heim_sig_gen_centered(int64_t radius, double step, HeimSigMapper mapper,
                                 HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, mapper);
  *out_sig = NULL;

  if (radius < 0 || radius > (INT64_MAX - 1) / 2 || (uint64_t)radius > ((uint64_t)SIZE_MAX - 1) / 2)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid radius");

  return heim_sig_gen_create(-radius, step, (size_t)radius * 2 + 1, mapper, out_sig);
}

HeimStatus heim_sig_gen_impulse(int64_t n_start, double step, size_t len, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig);
  *out_sig = NULL;

  HEIM_PROPAGATE_ERR(heim_sig_alloc(n_start, step, len, out_sig));
  (*out_sig)->i_data[0] = 1.0;
  return HEIM_OK;
}

HeimStatus heim_sig_gen_step(double step, size_t len, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig);
  *out_sig = NULL;

  HEIM_PROPAGATE_ERR(heim_sig_alloc(0, step, len, out_sig));
  for (size_t i = 0; i < len; i++)
    (*out_sig)->i_data[i] = 1.0;
  return HEIM_OK;
}

HeimStatus heim_sig_gen_exp(double amplitude, double omega_0, double phase, int64_t n_start,
                            size_t len, double step, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig);
  *out_sig = NULL;

  if (!isfinite(amplitude) || !isfinite(omega_0) || !isfinite(phase))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "exponential parameters must be finite");

  HEIM_PROPAGATE_ERR(heim_sig_alloc(n_start, step, len, out_sig));

  for (size_t i = 0; i < len; i++) {
    int64_t n = n_start + (int64_t)i;
    double angle = omega_0 * (double)n + phase;
    (*out_sig)->i_data[i] = amplitude * cos(angle);
    (*out_sig)->q_data[i] = amplitude * sin(angle);
  }
  return HEIM_OK;
}
