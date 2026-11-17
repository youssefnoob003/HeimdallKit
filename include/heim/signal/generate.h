#ifndef HEIM_SIGNAL_GENERATE_H
#define HEIM_SIGNAL_GENERATE_H

#include "heim/common.h"
#include "heim/signal/core.h"
#include "heim/status.h"

#include <stddef.h>
#include <stdint.h>

HEIM_BEGIN_EXTERN_C

/**
 * @brief Callback function type for analytical signal mapping.
 *
 * Evaluates in-phase (real) and quadrature (imaginary) values at continuous coordinate x = n *
 * step.
 *
 * @param x Continuous coordinate corresponding to discrete sample n (n * step).
 * @param[out] i Pointer to receive computed in-phase (real) value.
 * @param[out] q Pointer to receive computed quadrature (imaginary) value.
 */
typedef void (*HeimSigMapper)(double x, double *i, double *q);

/**
 * @brief Generates a signal by evaluating an analytical mapping callback at each sample.
 *
 * @param n_start Starting discrete sample index.
 * @param step Sample spacing.
 * @param len Number of samples to generate.
 * @param mapper Function callback to compute samples.
 * @param[out] out_sig Pointer to receive the generated signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments, or any error returned by
 * heim_sig_alloc().
 */
HeimStatus heim_sig_gen_create(int64_t n_start, double step, size_t len, HeimSigMapper mapper,
                               HeimSig **out_sig);

/**
 * @brief Generates a symmetric, zero-centered signal covering [-radius, +radius].
 *
 * Total signal length will be (2 * radius + 1) samples.
 *
 * @param radius Non-negative sample radius around n=0.
 * @param step Sample spacing.
 * @param mapper Function callback to compute samples.
 * @param[out] out_sig Pointer to receive the generated signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT if arguments are NULL or radius is
 * negative/overflows, or any error returned by heim_sig_gen_create().
 */
HeimStatus heim_sig_gen_centered(int64_t radius, double step, HeimSigMapper mapper,
                                 HeimSig **out_sig);

/**
 * @brief Generates a discrete unit impulse delta[n - n_start] (value 1.0 at index n_start, 0.0
 * elsewhere).
 *
 * @param n_start Discrete sample index where the impulse occurs.
 * @param step Sample spacing.
 * @param len Number of samples in finite support (must be >= 1).
 * @param[out] out_sig Pointer to receive the generated signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL out_sig, or any error returned by
 * heim_sig_alloc().
 */
HeimStatus heim_sig_gen_impulse(int64_t n_start, double step, size_t len, HeimSig **out_sig);

/**
 * @brief Generates a discrete unit step u[n] with support n in [0, len - 1].
 *
 * @param step Sample spacing.
 * @param len Number of samples in finite support.
 * @param[out] out_sig Pointer to receive the generated signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL out_sig, or any error returned by
 * heim_sig_alloc().
 */
HeimStatus heim_sig_gen_step(double step, size_t len, HeimSig **out_sig);

/**
 * @brief Generates a discrete complex exponential signal: amplitude * exp(j * (omega_0 * n +
 * phase)).
 *
 * @param amplitude Real scale factor.
 * @param omega_0 Angular frequency in radians per discrete sample.
 * @param phase Initial phase offset in radians.
 * @param n_start Starting discrete sample index.
 * @param len Number of samples in finite support.
 * @param step Sample spacing metadata.
 * @param[out] out_sig Pointer to receive the generated signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT if out_sig is NULL or amplitude/omega_0/phase
 * is non-finite, or any error returned by heim_sig_alloc().
 */
HeimStatus heim_sig_gen_exp(double amplitude, double omega_0, double phase, int64_t n_start,
                            size_t len, double step, HeimSig **out_sig);

HEIM_END_EXTERN_C

#endif
