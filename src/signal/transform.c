#include "heim/complex.h"
#include "heim/signal.h"
#include "heim/signal_internal.h"

#include <math.h>
#include <stdint.h>

HeimStatus heim_sig_xform_shift(const HeimSig *input, int64_t offset, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, input);
  *out_sig = NULL;
  if (!heim_sig_is_valid(input))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");

  HEIM_CHECK_ADD_OVERFLOW_I64(input->n_start, offset);

  HEIM_PROPAGATE_ERR(heim_sig_copy(input, out_sig));
  (*out_sig)->n_start += offset;
  return HEIM_OK;
}

HeimStatus heim_sig_xform_scale(const HeimSig *input, double factor, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, input);
  *out_sig = NULL;
  if (!heim_sig_is_valid(input) || !isfinite(factor))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid signal or non-finite factor");

  HEIM_PROPAGATE_ERR(heim_sig_copy(input, out_sig));
  for (size_t i = 0; i < input->len; i++) {
    (*out_sig)->i_data[i] *= factor;
    (*out_sig)->q_data[i] *= factor;
  }
  return HEIM_OK;
}

HeimStatus heim_sig_xform_scale_complex(const HeimSig *input, double factor_i, double factor_q,
                                        HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, input);
  *out_sig = NULL;
  if (!heim_sig_is_valid(input) || !isfinite(factor_i) || !isfinite(factor_q))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid signal or non-finite factor");

  HEIM_PROPAGATE_ERR(heim_sig_alloc(input->n_start, input->step, input->len, out_sig));

  HeimComplex factor = heim_complex(factor_i, factor_q);
  for (size_t i = 0; i < input->len; i++) {
    HeimComplex sample = heim_complex(input->i_data[i], input->q_data[i]);
    HeimComplex result = heim_complex_mul(sample, factor);
    (*out_sig)->i_data[i] = result.real;
    (*out_sig)->q_data[i] = result.imag;
  }
  return HEIM_OK;
}

HeimStatus heim_sig_xform_reverse(const HeimSig *input, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, input);
  *out_sig = NULL;
  if (!heim_sig_is_valid(input))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");

  int64_t end = heim_sig_end_unchecked(input);
  if (end == INT64_MIN)
    return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "reversal would overflow INT64_MIN");

  HEIM_PROPAGATE_ERR(heim_sig_alloc(-end, input->step, input->len, out_sig));
  for (size_t i = 0; i < input->len; i++) {
    (*out_sig)->i_data[i] = input->i_data[input->len - i - 1];
    (*out_sig)->q_data[i] = input->q_data[input->len - i - 1];
  }
  return HEIM_OK;
}
