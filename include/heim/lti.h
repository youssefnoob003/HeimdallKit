#ifndef HEIM_LTI_H
#define HEIM_LTI_H

#include "heim/common.h"
#include "heim/signal.h"

#include <stddef.h>

HEIM_BEGIN_EXTERN_C

/**
 * @brief Computes discrete-time linear convolution y[n] = (x * h)[n].
 *
 * @param x Input excitation signal.
 * @param h System impulse response signal.
 * @param[out] out_sig Pointer to receive the convolved output signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid signals,
 *         HEIM_INCOMPATIBLE if sampling steps differ, HEIM_OVERFLOW on index or length overflow,
 *         or any error returned by heim_sig_alloc().
 */
HeimStatus heim_lti_convolve(const HeimSig *x, const HeimSig *h, HeimSig **out_sig);

/**
 * @brief Checks if an LTI system is causal from its impulse response.
 *
 * A discrete LTI system is causal if and only if h[n] == 0 for all n < 0.
 *
 * @param h System impulse response.
 * @return True if impulse response is valid and causal, false otherwise.
 */
bool heim_lti_is_causal(const HeimSig *h);

/**
 * @brief Checks if an LTI system is memoryless (static) from its impulse response.
 *
 * A discrete LTI system is memoryless if and only if h[n] == 0 for all n != 0.
 *
 * @param h System impulse response.
 * @return True if impulse response is valid and memoryless, false otherwise.
 */
bool heim_lti_is_memoryless(const HeimSig *h);

/**
 * @brief Computes the step response s[n] of an LTI system from its impulse response h[n].
 *
 * Evaluates running cumulative sum s[n] = sum_{k=-inf}^n h[k] across the support of h.
 *
 * @param h System impulse response.
 * @param[out] out_sig Pointer to receive the step response signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid impulse response,
 *         or any error returned by heim_sig_alloc().
 */
HeimStatus heim_lti_step_response(const HeimSig *h, HeimSig **out_sig);

/**
 * @brief Filters an input signal using a Linear Constant-Coefficient Difference Equation (LCCDE).
 *
 * Implements Direct Form filtering:
 *   a[0]*y[n] = sum_{j=0}^{nb-1} b[j]*x[n-j] - sum_{j=1}^{na-1} a[j]*y[n-j]
 * assuming initial rest conditions (zero state prior to start of input signal).
 *
 * @param x Input excitation signal.
 * @param b Numerator feedforward filter coefficients array (length nb).
 * @param nb Number of feedforward coefficients (must be >= 1).
 * @param a Denominator feedback filter coefficients array (length na).
 * @param na Number of feedback coefficients (must be >= 1, a[0] != 0).
 * @param[out] out_sig Pointer to receive the filtered output signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT if arguments are NULL, signal is invalid,
 *         nb or na is 0, a[0] is zero/non-finite, or coefficients are non-finite,
 *         or any error returned by heim_sig_alloc().
 */
HeimStatus heim_lti_filter_lccde(const HeimSig *x, const double *b, size_t nb, const double *a,
                                 size_t na, HeimSig **out_sig);

HEIM_END_EXTERN_C

#endif
