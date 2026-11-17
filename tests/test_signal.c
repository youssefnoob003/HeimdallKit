#include "heim/heim.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>

static void linear_mapper(double x, double *i, double *q) {
  *i = x;
  *q = 2.0 * x;
}

static void centered_odd_mapper(double x, double *i, double *q) {
  *i = x;
  *q = 0.0;
}

static void centered_even_mapper(double x, double *i, double *q) {
  *i = x * x;
  *q = 0.0;
}

static HeimSig *create_signal(int64_t start, double step, size_t length, HeimSigMapper mapper) {
  HeimSig *sig = NULL;
  assert(HEIM_IS_OK(heim_sig_gen_create(start, step, length, mapper, &sig)));
  return sig;
}

static void test_create_shift_scale_reverse(void) {
  HeimSig *sig = create_signal(-1, 1.0, 3, linear_mapper);
  assert(heim_sig_start(sig) == -1);
  assert(heim_sig_end(sig) == 1);
  assert(heim_sig_len(sig) == 3);
  assert(heim_sig_end(NULL) == 0);
  double i, q;
  assert(HEIM_IS_OK(heim_sig_get_sample(sig, -1, &i, &q)));
  assert(i == -1.0 && q == -2.0);

  HeimSig *shifted = NULL;
  HeimSig *scaled = NULL;
  HeimSig *reversed = NULL;
  assert(HEIM_IS_OK(heim_sig_xform_shift(sig, 2, &shifted)));
  assert(HEIM_IS_OK(heim_sig_xform_scale(sig, 2.0, &scaled)));
  assert(HEIM_IS_OK(heim_sig_xform_reverse(sig, &reversed)));
  assert(heim_sig_start(shifted) == 1);
  assert(HEIM_IS_OK(heim_sig_get_sample(scaled, 1, &i, &q)));
  assert(i == 2.0 && q == 4.0);
  assert(heim_sig_start(reversed) == -1);
  assert(HEIM_IS_OK(heim_sig_get_sample(reversed, -1, &i, &q)));
  assert(i == 1.0);

  heim_sig_free(reversed);
  heim_sig_free(scaled);
  heim_sig_free(shifted);
  heim_sig_free(sig);
}

static void test_equal_parity_energy_power(void) {
  HeimSig *sig = create_signal(-2, 1.0, 5, linear_mapper);
  HeimSig *copy = NULL;
  HeimSig *odd = NULL;
  HeimSig *even = NULL;
  assert(HEIM_IS_OK(heim_sig_copy(sig, &copy)));
  assert(HEIM_IS_OK(heim_sig_gen_centered(2, 1.0, centered_odd_mapper, &odd)));
  assert(HEIM_IS_OK(heim_sig_gen_centered(2, 1.0, centered_even_mapper, &even)));
  assert(heim_sig_an_equal(sig, copy));
  assert(heim_sig_an_is_odd(odd));
  assert(heim_sig_an_is_even(even));
  double energy, power;
  assert(HEIM_IS_OK(heim_sig_an_energy(sig, &energy)));
  assert(fabs(energy - 50.0) < 1e-9);
  assert(HEIM_IS_OK(heim_sig_an_power(sig, 2, &power)));
  assert(fabs(power - 10.0) < 1e-9);

  HeimSig *single = NULL;
  assert(HEIM_IS_OK(heim_sig_alloc(0, 1.0, 1, &single)));
  assert(HEIM_IS_OK(heim_sig_set_sample(single, 0, 3.0, 0.0)));
  assert(HEIM_IS_OK(heim_sig_an_power(single, 1, &power)));
  assert(fabs(power - 3.0) < 1e-9);
  heim_sig_free(single);

  heim_sig_free(even);
  heim_sig_free(odd);
  heim_sig_free(copy);
  heim_sig_free(sig);
}

static void test_exp_add_multiply_combine(void) {
  HeimSig *exp1 = NULL;
  HeimSig *exp2 = NULL;
  HeimSig *sum = NULL;
  HeimSig *product = NULL;
  HeimSig *diff = NULL;
  assert(HEIM_IS_OK(heim_sig_gen_exp(2.0, 0.0, 0.0, 0, 2, 1.0, &exp1)));
  assert(HEIM_IS_OK(heim_sig_gen_exp(3.0, 0.0, 0.0, 0, 2, 1.0, &exp2)));
  assert(HEIM_IS_OK(heim_sig_op_add(exp1, exp2, &sum)));
  assert(HEIM_IS_OK(heim_sig_op_mul(exp1, exp2, &product)));
  assert(HEIM_IS_OK(heim_sig_op_combine(exp2, exp1, heim_complex_sub, &diff)));

  double i, q;
  assert(HEIM_IS_OK(heim_sig_get_sample(exp1, 0, &i, &q)));
  assert(fabs(i - 2.0) < 1e-9 && fabs(q) < 1e-9);
  assert(HEIM_IS_OK(heim_sig_get_sample(sum, 0, &i, &q)));
  assert(fabs(i - 5.0) < 1e-9 && fabs(q) < 1e-9);
  assert(HEIM_IS_OK(heim_sig_get_sample(product, 0, &i, &q)));
  assert(fabs(i - 6.0) < 1e-9 && fabs(q) < 1e-9);
  assert(HEIM_IS_OK(heim_sig_get_sample(diff, 0, &i, &q)));
  assert(fabs(i - 1.0) < 1e-9 && fabs(q) < 1e-9);

  heim_sig_free(diff);
  heim_sig_free(product);
  heim_sig_free(sum);
  heim_sig_free(exp2);
  heim_sig_free(exp1);
}

static void test_scale_complex(void) {
  HeimSig *sig = NULL;
  assert(HEIM_IS_OK(heim_sig_alloc(0, 1.0, 2, &sig)));
  assert(HEIM_IS_OK(heim_sig_set_sample(sig, 0, 3.0, 4.0)));
  assert(HEIM_IS_OK(heim_sig_set_sample(sig, 1, 1.0, -2.0)));

  /* Scale by (2 + 1j): (3+4j)(2+1j) = 2 + 11j, (1-2j)(2+1j) = 4 - 3j */
  HeimSig *scaled = NULL;
  assert(HEIM_IS_OK(heim_sig_xform_scale_complex(sig, 2.0, 1.0, &scaled)));
  double i, q;
  assert(HEIM_IS_OK(heim_sig_get_sample(scaled, 0, &i, &q)));
  assert(fabs(i - 2.0) < 1e-9 && fabs(q - 11.0) < 1e-9);
  assert(HEIM_IS_OK(heim_sig_get_sample(scaled, 1, &i, &q)));
  assert(fabs(i - 4.0) < 1e-9 && fabs(q + 3.0) < 1e-9);

  /* Invalid inputs. */
  HeimSig *bad = NULL;
  assert(heim_sig_xform_scale_complex(sig, NAN, 1.0, &bad).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_xform_scale_complex(sig, 1.0, INFINITY, &bad).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_xform_scale_complex(NULL, 1.0, 1.0, &bad).code == HEIM_INVALID_ARGUMENT);

  heim_sig_free(scaled);
  heim_sig_free(sig);
}

static void test_invalid_inputs_and_boundaries(void) {
  HeimSig *sig = NULL;
  assert(!heim_sig_is_valid(NULL));
  assert(heim_sig_alloc(0, 0.0, 1, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_alloc(0, NAN, 1, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_alloc(0, 1.0, 0, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_alloc(INT64_MAX, 1.0, 2, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_gen_impulse(0, 0.0, 1, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_gen_create(0, 1.0, 1, NULL, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_xform_shift(NULL, 1, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_xform_scale(NULL, 1.0, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(!heim_sig_an_equal(NULL, NULL));
  assert(!heim_sig_an_equal_overlap(NULL, NULL));
  double value;
  assert(heim_sig_an_energy(NULL, &value).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_io_write_raw(NULL, "/this/path/does/not/exist").code == HEIM_INVALID_ARGUMENT);

  assert(HEIM_IS_OK(heim_sig_alloc(-2, 1.0, 1, &sig)));
  HeimSig *right = NULL;
  assert(HEIM_IS_OK(heim_sig_alloc(2, 1.0, 1, &right)));
  assert(heim_sig_an_equal_overlap(sig, right));
  heim_sig_free(right);
  heim_sig_free(sig);

  HeimSig *near_zero = NULL;
  HeimSig *zero_outside = NULL;
  assert(HEIM_IS_OK(heim_sig_alloc(0, 1.0, 1, &near_zero)));
  assert(HEIM_IS_OK(heim_sig_alloc(1, 1.0, 1, &zero_outside)));
  assert(HEIM_IS_OK(heim_sig_set_sample(near_zero, 0, 1e-9, 0.0)));
  assert(heim_sig_an_equal(near_zero, zero_outside));
  assert(heim_sig_an_equal(zero_outside, near_zero));
  heim_sig_free(zero_outside);
  heim_sig_free(near_zero);
}

int main(void) {
  test_create_shift_scale_reverse();
  test_equal_parity_energy_power();
  test_exp_add_multiply_combine();
  test_scale_complex();
  test_invalid_inputs_and_boundaries();
  return 0;
}
