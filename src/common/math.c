#include "heim/math_internal.h"

#include <math.h>

bool heim_almost_equal_tol(double a, double b, double abs_tol, double rel_tol) {
  if (!isfinite(a) || !isfinite(b))
    return a == b;

  double difference = fabs(a - b);
  double scale = fmax(fabs(a), fabs(b));
  return difference <= abs_tol + rel_tol * scale;
}

bool heim_almost_equal(double a, double b) {
  return heim_almost_equal_tol(a, b, HEIM_DEFAULT_ABS_TOL, HEIM_DEFAULT_REL_TOL);
}
