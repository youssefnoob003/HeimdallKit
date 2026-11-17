#include "heim/common.h"
#include "heim/complex.h"
#include "heim/signal.h"
#include "heim/signal_internal.h"

HeimStatus heim_sig_op_combine(const HeimSig *sig1, const HeimSig *sig2, HeimSigOpCombiner combiner,
                               HeimSig **out_sig) {
  HEIM_CHECK_NOT_NULL(out_sig, sig1, sig2, combiner);
  *out_sig = NULL;

  if (!heim_sig_is_valid(sig1) || !heim_sig_is_valid(sig2))
    return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, "invalid input signal");
  if (!heim_sig_compatible(sig1, sig2))
    return HEIM_MAKE_ERR(HEIM_INCOMPATIBLE);

  int64_t end1 = heim_sig_end_unchecked(sig1);
  int64_t end2 = heim_sig_end_unchecked(sig2);
  int64_t out_start = HEIM_MIN(sig1->n_start, sig2->n_start);
  int64_t out_end = HEIM_MAX(end1, end2);
  size_t out_length;
  if (!heim_sig_range_length(out_start, out_end, &out_length))
    return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "combined range length overflow");

  HEIM_PROPAGATE_ERR(heim_sig_alloc(out_start, sig1->step, out_length, out_sig));

  for (size_t i = 0; i < out_length; i++) {
    int64_t n = out_start + (int64_t)i;
    HeimComplex s1 = heim_sig_get_sample_complex(sig1, n);
    HeimComplex s2 = heim_sig_get_sample_complex(sig2, n);
    HeimComplex res = combiner(s1, s2);
    (*out_sig)->i_data[i] = res.real;
    (*out_sig)->q_data[i] = res.imag;
  }
  return HEIM_OK;
}

HeimStatus heim_sig_op_add(const HeimSig *sig1, const HeimSig *sig2, HeimSig **out_sig) {
  return heim_sig_op_combine(sig1, sig2, heim_complex_add, out_sig);
}

HeimStatus heim_sig_op_mul(const HeimSig *sig1, const HeimSig *sig2, HeimSig **out_sig) {
  return heim_sig_op_combine(sig1, sig2, heim_complex_mul, out_sig);
}
