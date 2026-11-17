#include "heim/heim.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static void assert_same_samples(const HeimSig *expected, const HeimSig *actual) {
  assert(heim_sig_len(actual) == heim_sig_len(expected));
  assert(heim_sig_start(actual) == heim_sig_start(expected));
  assert(heim_sig_step(actual) == heim_sig_step(expected));
  for (size_t i = 0; i < heim_sig_len(expected); i++) {
    double expected_i, expected_q, actual_i, actual_q;
    int64_t n = heim_sig_start(expected) + (int64_t)i;
    assert(HEIM_IS_OK(heim_sig_get_sample(expected, n, &expected_i, &expected_q)));
    assert(HEIM_IS_OK(heim_sig_get_sample(actual, n, &actual_i, &actual_q)));
    if (isnan(expected_i))
      assert(isnan(actual_i));
    else
      assert(actual_i == expected_i);
    if (isnan(expected_q))
      assert(isnan(actual_q));
    else
      assert(actual_q == expected_q);
  }
}

static void test_binary_round_trip(void) {
  const char *path = "heim_test_signal.bin";
  const char *external_path = "heim_external_signal.bin";
  const char *metadata_path = "heim_test_signal.heim";
  HeimSig *original = NULL;
  assert(HEIM_IS_OK(heim_sig_alloc(-2, 0.25, 4, &original)));
  assert(HEIM_IS_OK(heim_sig_set_sample(original, -2, 1.0, -2.0)));
  assert(HEIM_IS_OK(heim_sig_set_sample(original, -1, 3.5, 4.25)));
  assert(HEIM_IS_OK(heim_sig_set_sample(original, 0, -INFINITY, NAN)));

  assert(HEIM_IS_OK(heim_sig_io_write_raw(original, path)));
  HeimSig *copy = NULL;
  assert(HEIM_IS_OK(
      heim_sig_io_read_raw(path, heim_sig_start(original), heim_sig_step(original), &copy)));
  assert_same_samples(original, copy);
  heim_sig_free(copy);

  FILE *external = fopen(external_path, "wb");
  assert(external != NULL);
  const double raw_samples[] = {10.0, 20.0, 30.0, 40.0};
  assert(fwrite(raw_samples, sizeof(raw_samples), 1, external) == 1);
  assert(fclose(external) == 0);
  HeimSig *external_copy = NULL;
  assert(HEIM_IS_OK(heim_sig_io_read_raw(external_path, 7, 0.5, &external_copy)));
  double i, q;
  assert(HEIM_IS_OK(heim_sig_get_sample(external_copy, 7, &i, &q)));
  assert(i == 10.0 && q == 20.0);
  assert(HEIM_IS_OK(heim_sig_get_sample(external_copy, 8, &i, &q)));
  assert(i == 30.0 && q == 40.0);
  heim_sig_free(external_copy);

  HeimSig *invalid_range = NULL;
  assert(heim_sig_io_read_raw(external_path, INT64_MAX, 0.5, &invalid_range).code ==
         HEIM_INVALID_ARGUMENT);
  assert(invalid_range == NULL);

  assert(HEIM_IS_OK(heim_sig_io_write_heim(original, metadata_path)));
  HeimSig *metadata_copy = NULL;
  assert(HEIM_IS_OK(heim_sig_io_read_heim(metadata_path, &metadata_copy)));
  assert_same_samples(original, metadata_copy);
  heim_sig_free(metadata_copy);

  heim_sig_free(original);
  assert(remove(path) == 0);
  assert(remove(external_path) == 0);
  assert(remove(metadata_path) == 0);
}

static void test_binary_errors(void) {
  const char *path = "heim_bad_signal.bin";
  FILE *fp = fopen(path, "wb");
  assert(fp != NULL);
  assert(fputs("bad", fp) >= 0);
  assert(fclose(fp) == 0);

  HeimSig *read = NULL;
  assert(heim_sig_io_read_raw("does-not-exist.bin", 0, 1.0, &read).code == HEIM_IO_ERROR);
  assert(heim_sig_io_read_raw(path, 0, 1.0, &read).code == HEIM_FORMAT_ERROR);
  assert(heim_sig_io_read_heim(path, &read).code == HEIM_FORMAT_ERROR);
  assert(heim_sig_io_read_raw(path, 0, 0.0, &read).code == HEIM_INVALID_ARGUMENT);
  assert(remove(path) == 0);
}

int main(void) {
  test_binary_round_trip();
  test_binary_errors();
  return 0;
}
