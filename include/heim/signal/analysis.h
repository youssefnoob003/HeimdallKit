#ifndef HEIM_SIGNAL_ANALYSIS_H
#define HEIM_SIGNAL_ANALYSIS_H

#include "heim/common.h"
#include "heim/signal/core.h"
#include "heim/status.h"

#include <stdbool.h>
#include <stdint.h>

HEIM_BEGIN_EXTERN_C

/**
 * @brief Checks if two signals are identically equal across their union support.
 *
 * Samples outside the stored support of either signal are treated as zero.
 *
 * @param sig1 First signal.
 * @param sig2 Second signal.
 * @return True if signals are compatible and have matching sample values across all indices, false
 * otherwise.
 */
bool heim_sig_an_equal(const HeimSig *sig1, const HeimSig *sig2);

/**
 * @brief Checks if two signals are equal only over their overlapping index range.
 *
 * If the signals do not overlap in time, returns true.
 *
 * @param sig1 First signal.
 * @param sig2 Second signal.
 * @return True if signals are compatible and match over their intersection support, false
 * otherwise.
 */
bool heim_sig_an_equal_overlap(const HeimSig *sig1, const HeimSig *sig2);

/**
 * @brief Checks if a signal exhibits odd symmetry (x[-n] == -x[n]).
 *
 * @param sig The signal to test.
 * @return True if the signal is valid and satisfies odd symmetry, false otherwise.
 */
bool heim_sig_an_is_odd(const HeimSig *sig);

/**
 * @brief Checks if a signal exhibits even symmetry (x[-n] == x[n]).
 *
 * @param sig The signal to test.
 * @return True if the signal is valid and satisfies even symmetry, false otherwise.
 */
bool heim_sig_an_is_even(const HeimSig *sig);

/**
 * @brief Computes total signal energy sum(|x[n]|^2) over a specified index range [first_n, last_n].
 *
 * @param sig The signal to evaluate.
 * @param first_n Starting discrete index (inclusive).
 * @param last_n Ending discrete index (inclusive).
 * @param[out] out_energy Pointer to receive computed energy.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments, invalid signal, or first_n >
 * last_n, or HEIM_OVERFLOW on range length overflow.
 */
HeimStatus heim_sig_an_energy_range(const HeimSig *sig, int64_t first_n, int64_t last_n,
                                    double *out_energy);

/**
 * @brief Computes total energy sum(|x[n]|^2) across the signal's full finite support.
 *
 * @param sig The signal to evaluate.
 * @param[out] out_energy Pointer to receive computed energy.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid signal,
 *         or any error returned by heim_sig_an_energy_range().
 */
HeimStatus heim_sig_an_energy(const HeimSig *sig, double *out_energy);

/**
 * @brief Computes average signal power energy / (last_n - first_n + 1) over [first_n, last_n].
 *
 * @param sig The signal to evaluate.
 * @param first_n Starting discrete index (inclusive).
 * @param last_n Ending discrete index (inclusive).
 * @param[out] out_power Pointer to receive computed average power.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or first_n > last_n,
 *         HEIM_OVERFLOW on sample count overflow, or any error returned by
 * heim_sig_an_energy_range().
 */
HeimStatus heim_sig_an_power_range(const HeimSig *sig, int64_t first_n, int64_t last_n,
                                   double *out_power);

/**
 * @brief Computes symmetric average signal power over [-radius, +radius].
 *
 * @param sig The signal to evaluate.
 * @param radius Symmetric sample radius (must be >= 0).
 * @param[out] out_power Pointer to receive computed average power.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT if out_power is NULL or radius < 0,
 *         HEIM_OVERFLOW if radius * 2 overflows, or any error returned by
 * heim_sig_an_power_range().
 */
HeimStatus heim_sig_an_power(const HeimSig *sig, int64_t radius, double *out_power);

HEIM_END_EXTERN_C

#endif
