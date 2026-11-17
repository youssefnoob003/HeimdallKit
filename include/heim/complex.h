#ifndef HEIM_COMPLEX_H
#define HEIM_COMPLEX_H

#include "heim/common.h"

#include <math.h>
#include <stdbool.h>

HEIM_BEGIN_EXTERN_C

/**
 * @brief Complex number representation with double-precision real and imaginary parts.
 */
typedef struct {
  double real; /**< Real component (I) */
  double imag; /**< Imaginary component (Q) */
} HeimComplex;

/**
 * @brief Constructs a complex number from real and imaginary parts.
 *
 * @param real The real component.
 * @param imag The imaginary component.
 * @return Constructed HeimComplex number.
 */
static inline HeimComplex heim_complex(double real, double imag) {
  HeimComplex c = {real, imag};
  return c;
}

/**
 * @brief Computes the sum of two complex numbers (a + b).
 *
 * @param a First complex operand.
 * @param b Second complex operand.
 * @return Sum of a and b.
 */
static inline HeimComplex heim_complex_add(HeimComplex a, HeimComplex b) {
  return (HeimComplex){a.real + b.real, a.imag + b.imag};
}

/**
 * @brief Computes the difference between two complex numbers (a - b).
 *
 * @param a Minuend complex operand.
 * @param b Subtrahend complex operand.
 * @return Difference a - b.
 */
static inline HeimComplex heim_complex_sub(HeimComplex a, HeimComplex b) {
  return (HeimComplex){a.real - b.real, a.imag - b.imag};
}

/**
 * @brief Computes the product of two complex numbers (a * b).
 *
 * @param a First complex operand.
 * @param b Second complex operand.
 * @return Product of a and b.
 */
static inline HeimComplex heim_complex_mul(HeimComplex a, HeimComplex b) {
  return (HeimComplex){
      a.real * b.real - a.imag * b.imag,
      a.real * b.imag + a.imag * b.real,
  };
}

/**
 * @brief Scales a complex number by a real scalar (a * factor).
 *
 * @param a The complex number to scale.
 * @param factor Real scaling scalar.
 * @return Scaled complex number.
 */
static inline HeimComplex heim_complex_scale(HeimComplex a, double factor) {
  return (HeimComplex){a.real * factor, a.imag * factor};
}

/**
 * @brief Computes the complex conjugate of a (real - j*imag).
 *
 * @param a Complex operand.
 * @return Conjugate of a.
 */
static inline HeimComplex heim_complex_conj(HeimComplex a) {
  return (HeimComplex){a.real, -a.imag};
}

/**
 * @brief Computes the squared magnitude of a complex number (|a|^2 = real^2 + imag^2).
 *
 * @param a Complex operand.
 * @return Squared magnitude.
 */
static inline double heim_complex_abs_sq(HeimComplex a) {
  return a.real * a.real + a.imag * a.imag;
}

/**
 * @brief Computes the magnitude (absolute value) of a complex number (|a| = sqrt(real^2 + imag^2)).
 *
 * @param a Complex operand.
 * @return Absolute magnitude.
 */
static inline double heim_complex_abs(HeimComplex a) {
  return sqrt(heim_complex_abs_sq(a));
}

/**
 * @brief Checks if both real and imaginary components are finite numbers.
 *
 * @param a Complex operand.
 * @return True if neither component is NaN or infinite, false otherwise.
 */
static inline bool heim_complex_is_finite(HeimComplex a) {
  return isfinite(a.real) && isfinite(a.imag);
}

HEIM_END_EXTERN_C

#endif
