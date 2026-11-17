#ifndef HEIM_INTERNAL_MATH_INTERNAL_H
#define HEIM_INTERNAL_MATH_INTERNAL_H

#include "heim/common.h"

#include <stdbool.h>

/**
 * @brief Checks if two floating-point numbers are equal within absolute and relative tolerances.
 *
 * Evaluates: fabs(a - b) <= abs_tol + rel_tol * max(fabs(a), fabs(b)).
 * Non-finite values compare equal only if bitwise identical (NaN == NaN is true).
 *
 * @param a First value.
 * @param b Second value.
 * @param abs_tol Absolute tolerance threshold.
 * @param rel_tol Relative tolerance scaling factor.
 * @return True if values are equal within tolerance, false otherwise.
 */
bool heim_almost_equal_tol(double a, double b, double abs_tol, double rel_tol);

/**
 * @brief Checks if two floating-point numbers are equal using default tolerances.
 *
 * Uses HEIM_DEFAULT_ABS_TOL (1e-9) and HEIM_DEFAULT_REL_TOL (1e-12).
 *
 * @param a First value.
 * @param b Second value.
 * @return True if values are almost equal, false otherwise.
 */
bool heim_almost_equal(double a, double b);

#endif
