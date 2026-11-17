#include "heim/heim.h"

#include <assert.h>
#include <math.h>

static HeimSig *signal_from_values(int64_t n_start, double step, size_t len, const double *i_values,
                                   const double *q_values) {
  HeimSig *sig = NULL;
  assert(HEIM_IS_OK(heim_sig_from_arrays(n_start, step, len, i_values, q_values, &sig)));
  return sig;
}

static void assert_close(double actual, double expected) {
  assert(fabs(actual - expected) < 1e-9);
}

static void assert_signal_values(const HeimSig *sig, int64_t n_start, double step, size_t len,
                                 const double *i_values, const double *q_values) {
  assert(sig != NULL);
  assert(heim_sig_start(sig) == n_start);
  assert(heim_sig_step(sig) == step);
  assert(heim_sig_len(sig) == len);

  for (size_t i = 0; i < len; i++) {
    double actual_i, actual_q;
    assert(HEIM_IS_OK(heim_sig_get_sample(sig, n_start + (int64_t)i, &actual_i, &actual_q)));
    assert_close(actual_i, i_values[i]);
    assert_close(actual_q, q_values[i]);
  }
}

static void test_convolve(void) {
  const double x_i[] = {1.0, 3.0};
  const double x_q[] = {2.0, 4.0};
  const double h_i[] = {5.0, 7.0};
  const double h_q[] = {6.0, 8.0};
  const double expected_i[] = {-7.0, -18.0, -11.0};
  const double expected_q[] = {16.0, 60.0, 52.0};

  HeimSig *x = signal_from_values(-1, 1.0, 2, x_i, x_q);
  HeimSig *h = signal_from_values(0, 1.0, 2, h_i, h_q);
  HeimSig *out = NULL;
  assert(HEIM_IS_OK(heim_lti_convolve(x, h, &out)));
  assert_signal_values(out, -1, 1.0, 3, expected_i, expected_q);

  HeimSig *different_step = NULL;
  assert(HEIM_IS_OK(heim_sig_alloc(0, 2.0, 1, &different_step)));
  HeimSig *invalid_out = NULL;
  assert(heim_lti_convolve(x, different_step, &invalid_out).code == HEIM_INCOMPATIBLE);

  heim_sig_free(different_step);
  heim_sig_free(out);
  heim_sig_free(h);
  heim_sig_free(x);
}

static void test_causality_and_memorylessness(void) {
  const double causal_i[] = {0.0, 0.0, 1.0, 2.0};
  const double causal_q[] = {0.0, 0.0, 0.0, 0.0};
  const double noncausal_i[] = {0.0, 1.0, 0.0, 0.0};
  const double memoryless_i[] = {0.0, 2.0, 0.0};
  const double nonmemoryless_i[] = {1.0, 2.0, 0.0};
  const double short_zero_q[] = {0.0, 0.0, 0.0};

  HeimSig *causal = signal_from_values(-2, 1.0, 4, causal_i, causal_q);
  HeimSig *noncausal = signal_from_values(-2, 1.0, 4, noncausal_i, causal_q);
  HeimSig *memoryless = signal_from_values(-1, 1.0, 3, memoryless_i, short_zero_q);
  HeimSig *nonmemoryless = signal_from_values(-1, 1.0, 3, nonmemoryless_i, short_zero_q);

  assert(heim_lti_is_causal(causal));
  assert(!heim_lti_is_causal(noncausal));
  assert(heim_lti_is_memoryless(memoryless));
  assert(!heim_lti_is_memoryless(nonmemoryless));

  heim_sig_free(nonmemoryless);
  heim_sig_free(memoryless);
  heim_sig_free(noncausal);
  heim_sig_free(causal);
}

static void test_step_response(void) {
  const double h_i[] = {1.0, 2.0, -1.0};
  const double h_q[] = {2.0, -1.0, 3.0};
  const double expected_i[] = {1.0, 3.0, 2.0};
  const double expected_q[] = {2.0, 1.0, 4.0};

  HeimSig *h = signal_from_values(-1, 0.5, 3, h_i, h_q);
  HeimSig *response = NULL;
  assert(HEIM_IS_OK(heim_lti_step_response(h, &response)));
  assert_signal_values(response, -1, 0.5, 3, expected_i, expected_q);

  heim_sig_free(response);
  heim_sig_free(h);
}

static void test_filter_lccde(void) {
  const double x_i[] = {1.0, 2.0, 3.0, 4.0};
  const double x_q[] = {1.0, 2.0, 3.0, 4.0};
  const double feedforward_i[] = {1.0, 2.5, 4.0, 5.5};
  const double feedforward_q[] = {1.0, 2.5, 4.0, 5.5};
  const double recursive_i[] = {1.0, 2.5, 4.25, 6.125};
  const double recursive_q[] = {1.0, 2.5, 4.25, 6.125};
  const double b_feedforward[] = {1.0, 0.5};
  const double b_recursive[] = {1.0};
  const double a_feedforward[] = {1.0};
  const double a_recursive[] = {1.0, -0.5};
  const double invalid_a[] = {0.0};

  HeimSig *x = signal_from_values(3, 1.0, 4, x_i, x_q);
  HeimSig *feedforward = NULL;
  HeimSig *recursive = NULL;
  assert(HEIM_IS_OK(heim_lti_filter_lccde(x, b_feedforward, 2, a_feedforward, 1, &feedforward)));
  assert(HEIM_IS_OK(heim_lti_filter_lccde(x, b_recursive, 1, a_recursive, 2, &recursive)));

  assert_signal_values(feedforward, 3, 1.0, 4, feedforward_i, feedforward_q);
  assert_signal_values(recursive, 3, 1.0, 4, recursive_i, recursive_q);
  HeimSig *invalid_out = NULL;
  assert(heim_lti_filter_lccde(x, b_recursive, 1, invalid_a, 1, &invalid_out).code ==
         HEIM_INVALID_ARGUMENT);

  heim_sig_free(recursive);
  heim_sig_free(feedforward);
  heim_sig_free(x);
}

static void test_lti_invalid_inputs(void) {
  const double coefficient = 1.0;
  HeimSig *x = NULL;
  HeimSig *out = NULL;
  assert(HEIM_IS_OK(heim_sig_alloc(0, 1.0, 1, &x)));
  assert(heim_lti_convolve(NULL, NULL, &out).code == HEIM_INVALID_ARGUMENT);
  assert(heim_lti_step_response(NULL, &out).code == HEIM_INVALID_ARGUMENT);
  assert(!heim_lti_is_causal(NULL));
  assert(!heim_lti_is_memoryless(NULL));
  assert(heim_lti_filter_lccde(NULL, &coefficient, 1, &coefficient, 1, &out).code ==
         HEIM_INVALID_ARGUMENT);
  assert(heim_lti_filter_lccde(x, NULL, 1, &coefficient, 1, &out).code == HEIM_INVALID_ARGUMENT);
  assert(heim_lti_filter_lccde(x, &coefficient, 0, &coefficient, 1, &out).code ==
         HEIM_INVALID_ARGUMENT);
  assert(heim_lti_filter_lccde(x, &coefficient, 1, NULL, 1, &out).code == HEIM_INVALID_ARGUMENT);
  assert(heim_lti_filter_lccde(x, &coefficient, 1, &coefficient, 0, &out).code ==
         HEIM_INVALID_ARGUMENT);
  const double nan_coefficient = NAN;
  assert(heim_lti_filter_lccde(x, &coefficient, 1, &nan_coefficient, 1, &out).code ==
         HEIM_INVALID_ARGUMENT);
  heim_sig_free(x);
}

int main(void) {
  test_convolve();
  test_causality_and_memorylessness();
  test_step_response();
  test_filter_lccde();
  test_lti_invalid_inputs();
  return 0;
}
