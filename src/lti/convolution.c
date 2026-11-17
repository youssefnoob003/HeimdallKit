#include "heim/complex.h"
#include "heim/lti.h"
#include "heim/signal_internal.h"

#include <stddef.h>
#include <stdint.h>

HeimStatus heim_lti_convolve(const HeimSig *x, const HeimSig *h, HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, x, h);
  *out_sig = NULL;

  if (!heim_sig_is_valid(x) || !heim_sig_is_valid(h))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");
  if (!heim_sig_compatible(x, h))
    return HEIM_MAKE_ERR(HEIM_INCOMPATIBLE);

  HEIM_CHECK_ADD_OVERFLOW_I64(x->n_start, h->n_start);
  HEIM_CHECK_OVERFLOW(h->len - 1 > SIZE_MAX - x->len);

  int64_t out_start = x->n_start + h->n_start;
  size_t out_length = x->len + h->len - 1;
  HEIM_PROPAGATE_ERR(heim_sig_alloc(out_start, x->step, out_length, out_sig));

  for (size_t xi = 0; xi < x->len; xi++) {
    HeimComplex x_val = heim_complex(x->i_data[xi], x->q_data[xi]);
    for (size_t hi = 0; hi < h->len; hi++) {
      HeimComplex h_val = heim_complex(h->i_data[hi], h->q_data[hi]);
      HeimComplex prod = heim_complex_mul(x_val, h_val);
      (*out_sig)->i_data[xi + hi] += prod.real;
      (*out_sig)->q_data[xi + hi] += prod.imag;
    }
  }
  return HEIM_OK;
}
