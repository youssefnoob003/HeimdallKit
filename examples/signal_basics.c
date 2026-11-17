/**
 * signal_basics.c - Fundamental discrete-time signal operations.
 *
 * Demonstrates:
 *   - Creating signals via analytical mappers (heim_sig_gen_centered)
 *   - Generating canonical signals (impulse, step, exponential)
 *   - Inspecting signal support, length, and sampling step
 *   - Computing signal energy and average power (heim_sig_an_energy, heim_sig_an_power)
 *   - Testing signal symmetry (even / odd parity)
 *   - Transformations: time-shifting, scaling, and reversal (heim_sig_xform_*)
 *   - Combining signals with custom operations using HeimComplex (heim_sig_op_combine)
 */

#include "heim/heim.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>

/* Mathematical callback for a complex pulse: s(t) = (1 + cos(t)) + j*sin(t) */
static void pulse_mapper(double t, double *i, double *q) {
  *i = 1.0 + cos(t);
  *q = sin(t);
}

/* Symmetrical even function: f(t) = t^2 */
static void even_mapper(double t, double *i, double *q) {
  *i = t * t;
  *q = 0.0;
}

/* Symmetrical odd function: f(t) = t */
static void odd_mapper(double t, double *i, double *q) {
  *i = t;
  *q = 0.0;
}

/* Custom sample combiner: computes 2*a + b */
static HeimComplex custom_linear_comb(HeimComplex a, HeimComplex b) {
  return heim_complex(2.0 * a.real + b.real, 2.0 * a.imag + b.imag);
}

static void print_sig_summary(const char *name, const HeimSig *sig) {
  printf("  %-16s: n in [%" PRId64 ", %" PRId64 "], len=%zu, step=%.2f\n", name,
         heim_sig_start(sig), heim_sig_end(sig), heim_sig_len(sig), heim_sig_step(sig));
}

int main(void) {
  printf("=== HeimdallKit: Signal Basics ===\n\n");

  /* 1. Create a custom signal from an analytical mapping function */
  printf("1. Creating analytic signals:\n");
  HeimSig *pulse = NULL;
  HeimStatus status = heim_sig_gen_centered(5, 0.1, pulse_mapper, &pulse);
  if (HEIM_IS_ERR(status)) {
    heim_status_print(status);
    return 1;
  }
  print_sig_summary("Centered Pulse", pulse);

  /* 2. Compute Energy and Power */
  double energy = 0.0;
  double power = 0.0;
  heim_sig_an_energy(pulse, &energy);
  heim_sig_an_power(pulse, 5, &power);
  printf("  Pulse Energy = %.4f, Average Power = %.4f\n\n", energy, power);

  /* 3. Generating canonical signals */
  printf("2. Canonical signal generation:\n");
  HeimSig *impulse = NULL;
  HeimSig *step_sig = NULL;
  HeimSig *complex_exp = NULL;

  heim_sig_gen_impulse(0, 1.0, 8, &impulse);
  heim_sig_gen_step(1.0, 8, &step_sig);
  /* Generate x[n] = 2.0 * exp(j * (pi/4 * n)) */
  heim_sig_gen_exp(2.0, HEIM_PI / 4.0, 0.0, 0, 8, 1.0, &complex_exp);

  print_sig_summary("Unit Impulse", impulse);
  print_sig_summary("Unit Step", step_sig);
  print_sig_summary("Complex Exp", complex_exp);
  printf("\n");

  /* 4. Symmetry / Parity tests */
  printf("3. Signal parity checks:\n");
  HeimSig *odd_sig = NULL;
  HeimSig *even_sig = NULL;
  heim_sig_gen_centered(4, 1.0, odd_mapper, &odd_sig);
  heim_sig_gen_centered(4, 1.0, even_mapper, &even_sig);

  printf("  odd_sig is_odd:  %s, is_even: %s\n", heim_sig_an_is_odd(odd_sig) ? "true" : "false",
         heim_sig_an_is_even(odd_sig) ? "true" : "false");
  printf("  even_sig is_odd: %s, is_even: %s\n", heim_sig_an_is_odd(even_sig) ? "true" : "false",
         heim_sig_an_is_even(even_sig) ? "true" : "false");
  printf("\n");

  /* 5. Transformations: shift, scale, reverse */
  printf("4. Signal transformations:\n");
  HeimSig *shifted = NULL;
  HeimSig *scaled = NULL;
  HeimSig *reversed = NULL;

  heim_sig_xform_shift(pulse, 3, &shifted);
  heim_sig_xform_scale(pulse, 0.5, &scaled);
  heim_sig_xform_reverse(pulse, &reversed);

  print_sig_summary("Original", pulse);
  print_sig_summary("Shifted (+3)", shifted);
  print_sig_summary("Scaled (0.5x)", scaled);
  print_sig_summary("Reversed", reversed);
  printf("\n");

  /* 6. Custom signal combination using HeimComplex callback */
  printf("5. Combining signals (custom 2*A + B):\n");
  HeimSig *comb = NULL;
  heim_sig_op_combine(impulse, step_sig, custom_linear_comb, &comb);
  print_sig_summary("2*Impulse + Step", comb);
  for (size_t i = 0; i < 4; i++) {
    double re, im;
    heim_sig_get_sample(comb, (int64_t)i, &re, &im);
    printf("    n=%zu: %.1f + %.1fj\n", i, re, im);
  }

  /* Cleanup */
  heim_sig_free(comb);
  heim_sig_free(reversed);
  heim_sig_free(scaled);
  heim_sig_free(shifted);
  heim_sig_free(even_sig);
  heim_sig_free(odd_sig);
  heim_sig_free(complex_exp);
  heim_sig_free(step_sig);
  heim_sig_free(impulse);
  heim_sig_free(pulse);

  printf("\nAll signal basic operations executed successfully.\n");
  return 0;
}
