#ifndef HEIM_SIGNAL_OPERATIONS_H
#define HEIM_SIGNAL_OPERATIONS_H

#include "heim/common.h"
#include "heim/complex.h"
#include "heim/signal/core.h"
#include "heim/status.h"

HEIM_BEGIN_EXTERN_C

/**
 * @brief Callback function type for combining two complex sample values.
 *
 * @param a Sample value from first signal (or 0 if outside support).
 * @param b Sample value from second signal (or 0 if outside support).
 * @return Resulting combined complex sample value.
 */
typedef HeimComplex (*HeimSigOpCombiner)(HeimComplex a, HeimComplex b);

/**
 * @brief Combines two signals point-by-point over their union support using a custom combiner
 * function.
 *
 * @param sig1 First operand signal.
 * @param sig2 Second operand signal.
 * @param combiner Function callback defining the binary sample combination.
 * @param[out] out_sig Pointer to receive the combined signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments or invalid signals,
 *         HEIM_INCOMPATIBLE if sampling steps differ, HEIM_OVERFLOW on union range overflow,
 *         or any error returned by heim_sig_alloc().
 */
HeimStatus heim_sig_op_combine(const HeimSig *sig1, const HeimSig *sig2, HeimSigOpCombiner combiner,
                               HeimSig **out_sig);

/**
 * @brief Computes point-by-point sum of two signals: y[n] = sig1[n] + sig2[n].
 *
 * Evaluated across the union support of both signals.
 *
 * @param sig1 First operand signal.
 * @param sig2 Second operand signal.
 * @param[out] out_sig Pointer to receive the summed signal. Caller assumes ownership.
 * @return HEIM_OK on success, or any error returned by heim_sig_op_combine().
 */
HeimStatus heim_sig_op_add(const HeimSig *sig1, const HeimSig *sig2, HeimSig **out_sig);

/**
 * @brief Computes point-by-point product of two signals: y[n] = sig1[n] * sig2[n].
 *
 * Evaluated across the union support of both signals.
 *
 * @param sig1 First operand signal.
 * @param sig2 Second operand signal.
 * @param[out] out_sig Pointer to receive the multiplied signal. Caller assumes ownership.
 * @return HEIM_OK on success, or any error returned by heim_sig_op_combine().
 */
HeimStatus heim_sig_op_mul(const HeimSig *sig1, const HeimSig *sig2, HeimSig **out_sig);

HEIM_END_EXTERN_C

#endif
