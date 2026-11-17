#ifndef HEIM_COMMON_H
#define HEIM_COMMON_H

/* Public headers are usable from C++ without changing the C ABI. */
#ifdef __cplusplus
#define HEIM_BEGIN_EXTERN_C extern "C" {
#define HEIM_END_EXTERN_C }
#else
#define HEIM_BEGIN_EXTERN_C
#define HEIM_END_EXTERN_C
#endif

/* Common numerical constants */
#ifndef HEIM_PI
#define HEIM_PI 3.14159265358979323846
#endif

#ifndef HEIM_DEFAULT_ABS_TOL
#define HEIM_DEFAULT_ABS_TOL 1e-9
#endif

#ifndef HEIM_DEFAULT_REL_TOL
#define HEIM_DEFAULT_REL_TOL 1e-12
#endif

/* Utility macros */
#define HEIM_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HEIM_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define HEIM_CLAMP(x, low, high) (HEIM_MIN(HEIM_MAX((x), (low)), (high)))

#endif
