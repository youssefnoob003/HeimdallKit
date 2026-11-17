/**
 * matched_filter.c - Radar / Communications Matched Filter & Signal I/O.
 *
 * Demonstrates:
 *   - Pulse compression / matched filtering for optimal SNR in radar/comms
 *   - Constructing the matched filter impulse response h[n] = s*[-n] (heim_sig_xform_reverse)
 *   - Detecting peak correlation to determine signal arrival delay
 *   - Exporting and importing signal data with metadata using heim_sig_io_write_heim
 */

#include "heim/heim.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>

int main(void) {
  printf("=== HeimdallKit: Radar / RF Matched Filter ===\n\n");

  /* 1. Define a known transmitted pulse template s[n] (5-element Barker-like sequence) */
  const double tx_i[] = {1.0, 1.0, 1.0, -1.0, 1.0};
  const double tx_q[] = {0.0, 0.0, 0.0, 0.0, 0.0};
  const size_t tx_len = 5;

  HeimSig *tx_pulse = NULL;
  HeimStatus status = heim_sig_from_arrays(0, 1.0, tx_len, tx_i, tx_q, &tx_pulse);
  if (HEIM_IS_ERR(status)) {
    heim_status_print(status);
    return 1;
  }
  printf("1. Transmitted reference pulse s[n] (length %zu):\n", tx_len);
  for (size_t i = 0; i < tx_len; i++) {
    printf("   s[%zu] = %+.1f\n", i, tx_i[i]);
  }

  /*
   * 2. Synthesize a received signal r[n] where the pulse arrives with delay n0 = 6.
   *    Total buffer size = 16 samples.
   */
  const size_t rx_len = 16;
  const int64_t target_delay = 6;
  HeimSig *rx_signal = NULL;
  heim_sig_alloc(0, 1.0, rx_len, &rx_signal);

  /* Insert delayed pulse into received channel with background noise */
  for (size_t i = 0; i < tx_len; i++) {
    heim_sig_set_sample(rx_signal, target_delay + (int64_t)i, tx_i[i], 0.0);
  }
  printf("\n2. Simulated channel: Pulse arriving at delay n = %" PRId64 "\n", target_delay);

  /*
   * 3. Construct the matched filter impulse response:
   *    h[n] = s*[-n] (time-reversed complex conjugate of reference pulse)
   */
  HeimSig *matched_filter = NULL;
  heim_sig_xform_reverse(tx_pulse, &matched_filter);

  /*
   * 4. Convolve received signal with matched filter:
   *    y[n] = r[n] * h[n]
   */
  HeimSig *output = NULL;
  heim_lti_convolve(rx_signal, matched_filter, &output);

  /* 5. Peak detection: find the index of maximum energy / correlation */
  int64_t peak_index = 0;
  double peak_energy = -1.0;
  size_t out_len = heim_sig_len(output);
  int64_t out_start = heim_sig_start(output);

  printf("\n3. Matched filter output correlation:\n");
  for (size_t i = 0; i < out_len; i++) {
    int64_t n = out_start + (int64_t)i;
    HeimComplex c = heim_sig_get_sample_complex(output, n);
    double energy = heim_complex_abs_sq(c);
    printf("   n=%2" PRId64 ": energy = %6.2f", n, energy);
    if (energy > peak_energy) {
      peak_energy = energy;
      peak_index = n;
    }
    int bar = (int)(energy * 1.5);
    printf(" |");
    for (int b = 0; b < bar; b++)
      putchar('*');
    putchar('\n');
  }

  printf("\n  >>> Detected correlation peak at n = %" PRId64 " (energy = %.2f)\n", peak_index,
         peak_energy);
  if (peak_index == target_delay) {
    printf("  >>> Signal detection PASSED: target delay successfully resolved!\n\n");
  } else {
    printf("  >>> Signal detection FAILED.\n\n");
  }

  /* 6. Demonstrate binary format I/O: save correlation output to disk */
  const char *dump_path = "matched_filter_output.heim";
  printf("4. Saving filter output to self-describing binary file: %s\n", dump_path);
  status = heim_sig_io_write_heim(output, dump_path);
  if (HEIM_IS_OK(status)) {
    printf("   Successfully written.\n");
    HeimSig *loaded = NULL;
    status = heim_sig_io_read_heim(dump_path, &loaded);
    if (HEIM_IS_OK(status)) {
      printf("   Successfully read back %zu samples (start=%" PRId64 ", step=%.1f)\n",
             heim_sig_len(loaded), heim_sig_start(loaded), heim_sig_step(loaded));
      heim_sig_free(loaded);
    }
    remove(dump_path);
  }

  /* Cleanup */
  heim_sig_free(output);
  heim_sig_free(matched_filter);
  heim_sig_free(rx_signal);
  heim_sig_free(tx_pulse);

  printf("\nMatched filter demonstration complete.\n");
  return 0;
}
