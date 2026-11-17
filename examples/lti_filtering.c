/**
 * lti_filtering.c - Linear Time-Invariant (LTI) systems and filtering.
 *
 * Demonstrates:
 *   - Defining an LTI system by its impulse response h[n]
 *   - Checking causality and memorylessness of an LTI system (heim_lti_is_causal)
 *   - Computing system response via discrete-time convolution (heim_lti_convolve)
 *   - Computing the system step response (heim_lti_step_response)
 *   - Applying Linear Constant-Coefficient Difference Equation (LCCDE) filtering
 *     (heim_lti_filter_lccde)
 */

#include "heim/heim.h"

#include <inttypes.h>
#include <stdio.h>

static void print_signal_values(const char *label, const HeimSig *sig) {
  printf("%s:\n", label);
  for (size_t i = 0; i < heim_sig_len(sig); i++) {
    int64_t n = heim_sig_start(sig) + (int64_t)i;
    double re, im;
    heim_sig_get_sample(sig, n, &re, &im);
    printf("  n=%2" PRId64 ": %7.3f + %7.3fj\n", n, re, im);
  }
}

int main(void) {
  printf("=== HeimdallKit: LTI Systems & Filtering ===\n\n");

  /* 1. Define an impulse response h[n] for a causal 3-tap FIR filter */
  printf("1. FIR Filter Impulse Response h[n]:\n");
  const double h_i[] = {0.25, 0.5, 0.25}; /* 3-point triangular / weighted average */
  const double h_q[] = {0.0, 0.0, 0.0};
  HeimSig *h = NULL;
  HeimStatus status = heim_sig_from_arrays(0, 1.0, 3, h_i, h_q, &h);
  if (HEIM_IS_ERR(status)) {
    heim_status_print(status);
    return 1;
  }
  print_signal_values("h[n]", h);

  /* Check LTI system properties */
  printf("\nSystem Properties:\n");
  printf("  is_causal:       %s\n", heim_lti_is_causal(h) ? "true" : "false");
  printf("  is_memoryless:   %s\n", heim_lti_is_memoryless(h) ? "true" : "false");

  /* 2. Compute the step response of the FIR system */
  printf("\n2. FIR Step Response s[n] = sum_{k=-inf}^n h[k]:\n");
  HeimSig *step_resp = NULL;
  heim_lti_step_response(h, &step_resp);
  print_signal_values("s[n]", step_resp);

  /* 3. Convolve an input signal x[n] with h[n] */
  printf("\n3. Discrete-Time Convolution y[n] = x[n] * h[n]:\n");
  const double x_i[] = {1.0, 2.0, 3.0, 2.0, 1.0};
  const double x_q[] = {0.0, 0.0, 0.0, 0.0, 0.0};
  HeimSig *x = NULL;
  heim_sig_from_arrays(0, 1.0, 5, x_i, x_q, &x);

  HeimSig *y_conv = NULL;
  heim_lti_convolve(x, h, &y_conv);
  print_signal_values("Input x[n]", x);
  print_signal_values("Output y[n]", y_conv);

  /* 4. Recursive IIR Filtering via LCCDE Difference Equation */
  /*
   * First-order low-pass filter (leaky integrator):
   *   y[n] - 0.6 * y[n-1] = 0.4 * x[n]
   *   a = [1.0, -0.6]  (denominator / feedback)
   *   b = [0.4]        (numerator / feedforward)
   */
  printf("\n4. Recursive IIR Filter (y[n] = 0.6 y[n-1] + 0.4 x[n]):\n");
  const double a[] = {1.0, -0.6};
  const double b[] = {0.4};

  /* Input: step signal of length 8 */
  HeimSig *unit_step = NULL;
  heim_sig_gen_step(1.0, 8, &unit_step);

  HeimSig *y_iir = NULL;
  heim_lti_filter_lccde(unit_step, b, 1, a, 2, &y_iir);
  print_signal_values("IIR Step Response", y_iir);

  /* Cleanup */
  heim_sig_free(y_iir);
  heim_sig_free(unit_step);
  heim_sig_free(y_conv);
  heim_sig_free(x);
  heim_sig_free(step_resp);
  heim_sig_free(h);

  printf("\nLTI filtering operations completed successfully.\n");
  return 0;
}
