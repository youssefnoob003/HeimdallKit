#include "heim/lti.h"
#include "heim/math_internal.h"
#include "heim/signal_internal.h"

bool heim_lti_is_causal(const HeimSig *h) {
  if (!heim_sig_is_valid(h))
    return false;

  for (size_t i = 0; i < h->len; i++) {
    int64_t n = h->n_start + (int64_t)i;
    if (n >= 0)
      break;
    if (!heim_almost_equal(h->i_data[i], 0.0) || !heim_almost_equal(h->q_data[i], 0.0))
      return false;
  }
  return true;
}

bool heim_lti_is_memoryless(const HeimSig *h) {
  if (!heim_sig_is_valid(h))
    return false;

  for (size_t i = 0; i < h->len; i++) {
    int64_t n = h->n_start + (int64_t)i;
    if (n != 0 && (!heim_almost_equal(h->i_data[i], 0.0) || !heim_almost_equal(h->q_data[i], 0.0)))
      return false;
  }
  return true;
}

HeimStatus heim_lti_step_response(const HeimSig *h, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, h);
  *out_sig = NULL;
  if (!heim_sig_is_valid(h))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid impulse response");

  HEIM_PROPAGATE_ERR(heim_sig_alloc(h->n_start, h->step, h->len, out_sig));

  (*out_sig)->i_data[0] = h->i_data[0];
  (*out_sig)->q_data[0] = h->q_data[0];
  for (size_t i = 1; i < h->len; i++) {
    (*out_sig)->i_data[i] = (*out_sig)->i_data[i - 1] + h->i_data[i];
    (*out_sig)->q_data[i] = (*out_sig)->q_data[i - 1] + h->q_data[i];
  }
  return HEIM_OK;
}
