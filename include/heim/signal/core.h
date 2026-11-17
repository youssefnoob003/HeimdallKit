#ifndef HEIM_SIGNAL_CORE_H
#define HEIM_SIGNAL_CORE_H

#include "heim/common.h"
#include "heim/complex.h"
#include "heim/status.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

HEIM_BEGIN_EXTERN_C

/*
 * HeimSig owns its I/Q sample arrays.
 * Successful functions writing to HeimSig ** transfer ownership to the caller,
 * who must free it with heim_sig_free().
 */
typedef struct HeimSig HeimSig;

/**
 * @brief Allocates an uninitialized discrete-time signal with zeroed sample arrays.
 *
 * @param n_start Starting discrete sample index.
 * @param step Positive sample spacing/period.
 * @param len Number of samples in finite support.
 * @param[out] out_sig Pointer to receive the allocated signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on invalid parameters or range overflow,
 *         HEIM_OVERFLOW if length exceeds addressable memory, or HEIM_ALLOCATION_FAILED on memory
 * allocation failure.
 */
HeimStatus heim_sig_alloc(int64_t n_start, double step, size_t len, HeimSig **out_sig);

/**
 * @brief Allocates a signal and copies sample data from external I and Q arrays.
 *
 * @param n_start Starting discrete sample index.
 * @param step Positive sample spacing/period.
 * @param len Number of samples to copy.
 * @param i_values Array of in-phase (real) sample values.
 * @param q_values Array of quadrature (imaginary) sample values.
 * @param[out] out_sig Pointer to receive the allocated signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL arguments, or any error returned by
 * heim_sig_alloc().
 */
HeimStatus heim_sig_from_arrays(int64_t n_start, double step, size_t len, const double *i_values,
                                const double *q_values, HeimSig **out_sig);

/**
 * @brief Creates a deep copy of an existing signal.
 *
 * @param sig The source signal to copy.
 * @param[out] out_sig Pointer to receive the newly cloned signal. Caller assumes ownership.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT if the input signal is NULL or invalid,
 *         or any error returned by heim_sig_from_arrays().
 */
HeimStatus heim_sig_copy(const HeimSig *sig, HeimSig **out_sig);

/**
 * @brief Releases memory associated with a signal and its internal sample arrays.
 *
 * @param sig The signal to free (safe to call with NULL).
 */
void heim_sig_free(HeimSig *sig);

/**
 * @brief Validates signal structural invariants and internal allocations.
 *
 * @param sig The signal to validate.
 * @return True if non-NULL, arrays are allocated, range does not overflow, and step is finite and >
 * 0; false otherwise.
 */
bool heim_sig_is_valid(const HeimSig *sig);

/**
 * @brief Returns the length of the signal in samples.
 *
 * @param sig The signal to inspect.
 * @return Number of samples, or 0 if signal is invalid.
 */
size_t heim_sig_len(const HeimSig *sig);

/**
 * @brief Returns the starting discrete sample index n_start.
 *
 * @param sig The signal to inspect.
 * @return Starting sample index, or 0 if signal is invalid.
 */
int64_t heim_sig_start(const HeimSig *sig);

/**
 * @brief Returns the ending discrete sample index (n_start + len - 1).
 *
 * @param sig The signal to inspect.
 * @return Ending sample index, or 0 if signal is invalid.
 */
int64_t heim_sig_end(const HeimSig *sig);

/**
 * @brief Returns the sample step spacing.
 *
 * @param sig The signal to inspect.
 * @return Sample step spacing, or 0.0 if signal is invalid.
 */
double heim_sig_step(const HeimSig *sig);

/**
 * @brief Checks if a discrete index n falls within the signal's stored support [n_start, end].
 *
 * @param sig The signal to inspect.
 * @param n Discrete sample index to query.
 * @return True if n is inside the finite support, false otherwise.
 */
bool heim_sig_contains(const HeimSig *sig, int64_t n);

/**
 * @brief Retrieves in-phase and quadrature values at discrete sample index n.
 *
 * If n is outside the stored support, writes 0.0 to *i and *q and returns HEIM_OUT_OF_RANGE.
 *
 * @param sig The signal to query.
 * @param n Discrete sample index.
 * @param[out] i Pointer to receive real/I component.
 * @param[out] q Pointer to receive imaginary/Q component.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL pointers or invalid signal,
 *         or HEIM_OUT_OF_RANGE if n is outside finite support.
 */
HeimStatus heim_sig_get_sample(const HeimSig *sig, int64_t n, double *i, double *q);

/**
 * @brief Sets in-phase and quadrature values at discrete sample index n.
 *
 * @param sig The signal to modify.
 * @param n Discrete sample index within stored support.
 * @param i New in-phase (real) value.
 * @param q New quadrature (imaginary) value.
 * @return HEIM_OK on success, HEIM_INVALID_ARGUMENT on NULL pointer or invalid signal,
 *         or HEIM_OUT_OF_RANGE if n is outside finite support.
 */
HeimStatus heim_sig_set_sample(HeimSig *sig, int64_t n, double i, double q);

/**
 * @brief Retrieves sample at discrete index n as a HeimComplex value.
 *
 * @param sig The signal to query.
 * @param n Discrete sample index.
 * @return Sample value as HeimComplex, or {0.0, 0.0} if n is outside finite support or signal is
 * invalid.
 */
HeimComplex heim_sig_get_sample_complex(const HeimSig *sig, int64_t n);

/**
 * @brief Checks if two signals have identical sampling steps within numerical tolerance.
 *
 * @param sig1 First signal.
 * @param sig2 Second signal.
 * @return True if both signals are valid and share the same step, false otherwise.
 */
bool heim_sig_compatible(const HeimSig *sig1, const HeimSig *sig2);

HEIM_END_EXTERN_C

#endif
