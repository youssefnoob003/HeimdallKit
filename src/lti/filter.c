#include "heim/lti.h"
#include "heim/signal_internal.h"

#include <math.h>

HeimStatus heim_lti_filter_lccde(const HeimSig *x, const double *b, size_t nb, const double *a,
                                 size_t na, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, x, b, a);
  *out_sig = NULL;

  if (!heim_sig_is_valid(x) || nb == 0 || na == 0 || a[0] == 0.0)
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid filter parameters or zero a[0]");

  for (size_t i = 0; i < nb; i++) {
    if (!isfinite(b[i]))
      return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "non-finite coefficient in b");
  }
  for (size_t i = 0; i < na; i++) {
    if (!isfinite(a[i]))
      return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "non-finite coefficient in a");
  }

  HEIM_PROPAGATE_ERR(heim_sig_alloc(x->n_start, x->step, x->len, out_sig));

  double inverse_a0 = 1.0 / a[0];
  for (size_t i = 0; i < x->len; i++) {
    for (size_t j = 0; j < nb && j <= i; j++) {
      (*out_sig)->i_data[i] += inverse_a0 * b[j] * x->i_data[i - j];
      (*out_sig)->q_data[i] += inverse_a0 * b[j] * x->q_data[i - j];
    }

    for (size_t j = 1; j < na && j <= i; j++) {
      (*out_sig)->i_data[i] -= inverse_a0 * a[j] * (*out_sig)->i_data[i - j];
      (*out_sig)->q_data[i] -= inverse_a0 * a[j] * (*out_sig)->q_data[i - j];
    }
  }
  return HEIM_OK;
}
