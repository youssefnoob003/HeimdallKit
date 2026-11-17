#ifndef HEIM_SIGNAL_TRANSFORM_H
#define HEIM_SIGNAL_TRANSFORM_H

#include "heim/common.h"
#include "heim/complex.h"
#include "heim/signal/core.h"
#include "heim/status.h"

#include <stdint.h>

HEIM_BEGIN_EXTERN_C

/**
 * @brief Time-shifts a signal by an integer offset: y[n] = x[n - offset].
 *
 * Shifts the starting index to (n_start + offset) without altering sample values.
 *
 * @param input Source signal to shift.
 * @param offset Integer sample shift (positive shifts right/delays, negative shifts left/advances).
 * @param[out] out_sig Pointer to receive the shifted signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid signal,
 *         HEIM_OVERFLOW on index overflow, or any error returned by heim_sig_copy().
 */
HeimStatus heim_sig_xform_shift(const HeimSig *input, int64_t offset, HeimSig **out_sig);

/**
 * @brief Scales all sample amplitudes by a real scalar: y[n] = factor * x[n].
 *
 * @param input Source signal to scale.
 * @param factor Real scaling factor (must be finite).
 * @param[out] out_sig Pointer to receive the scaled signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments, invalid signal, or
 * non-finite factor, or any error returned by heim_sig_copy().
 */
HeimStatus heim_sig_xform_scale(const HeimSig *input, double factor, HeimSig **out_sig);

/**
 * @brief Scales all sample amplitudes by a complex scalar: y[n] = (factor_i + j*factor_q) * x[n].
 *
 * @param input Source signal to scale.
 * @param factor_i Real component of complex factor (must be finite).
 * @param factor_q Imaginary component of complex factor (must be finite).
 * @param[out] out_sig Pointer to receive the scaled signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments, invalid signal, or
 * non-finite factors, or any error returned by heim_sig_alloc().
 */
HeimStatus heim_sig_xform_scale_complex(const HeimSig *input, double factor_i, double factor_q,
                                        HeimSig **out_sig);

/**
 * @brief Time-reverses a signal: y[n] = x[-n].
 *
 * New support becomes [-end, -n_start].
 *
 * @param input Source signal to reverse.
 * @param[out] out_sig Pointer to receive the reversed signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid signal,
 *         HEIM_OVERFLOW if reversal overflows INT64_MIN, or any error returned by heim_sig_alloc().
 */
HeimStatus heim_sig_xform_reverse(const HeimSig *input, HeimSig **out_sig);

HEIM_END_EXTERN_C

#endif
