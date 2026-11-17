#ifndef HEIM_STATUS_H
#define HEIM_STATUS_H

#include "heim/common.h"

#include <stdint.h>
#include <stdio.h>

HEIM_BEGIN_EXTERN_C

typedef enum {
  HEIM_SUCCESS = 0,
  HEIM_INVALID_ARGUMENT,
  HEIM_OUT_OF_RANGE,
  HEIM_INCOMPATIBLE,
  HEIM_ALLOCATION_FAILED,
  HEIM_IO_ERROR,
  HEIM_FORMAT_ERROR,
  HEIM_OVERFLOW
} HeimErrorCode;

typedef struct HeimStatus {
  HeimErrorCode code;
  const char *file;
  int line;
  const char *context;
} HeimStatus;

/* Success status constant */
#define HEIM_OK ((HeimStatus){.code = HEIM_SUCCESS, .file = NULL, .line = 0, .context = NULL})

/* Status querying macros */
#define HEIM_IS_OK(s) ((s).code == HEIM_SUCCESS)
#define HEIM_IS_ERR(s) ((s).code != HEIM_SUCCESS)

/* Error construction macros */
#define HEIM_MAKE_ERR(err)                                                                         \
  ((HeimStatus){.code = (err), .file = __FILE__, .line = __LINE__, .context = NULL})

#define HEIM_MAKE_ERR_CTX(err, ctx)                                                                \
  ((HeimStatus){.code = (err), .file = __FILE__, .line = __LINE__, .context = (ctx)})

/* Error propagation macro */
#define HEIM_PROPAGATE_ERR(expr)                                                                   \
  do {                                                                                             \
    HeimStatus _status = (expr);                                                                   \
    if (HEIM_IS_ERR(_status))                                                                      \
      return _status;                                                                              \
  } while (0)

/* Overflow check macros */
#define HEIM_CHECK_OVERFLOW(cond)                                                                  \
  do {                                                                                             \
    if (cond)                                                                                      \
      return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, #cond);                                              \
  } while (0)

#define HEIM_CHECK_ADD_OVERFLOW_I64(a, b)                                                          \
  do {                                                                                             \
    uint64_t _heim_ua = (uint64_t)(int64_t)(a);                                                    \
    uint64_t _heim_ub = (uint64_t)(int64_t)(b);                                                    \
    uint64_t _heim_usum = _heim_ua + _heim_ub;                                                     \
    if ((((_heim_ua ^ _heim_usum) & (_heim_ub ^ _heim_usum)) & 0x8000000000000000ULL) != 0)       \
      return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "int64 add overflow: " #a " + " #b);                 \
  } while (0)

#define HEIM_CHECK_ADD_OVERFLOW_SIZE(a, b)                                                         \
  do {                                                                                             \
    size_t _heim_sa = (size_t)(a);                                                                 \
    size_t _heim_sb = (size_t)(b);                                                                 \
    if (_heim_sb > SIZE_MAX - _heim_sa)                                                            \
      return HEIM_MAKE_ERR_CTX(HEIM_OVERFLOW, "size_t add overflow: " #a " + " #b);                \
  } while (0)

/* Variadic NULL checks that automatically report which field is NULL */
#define HEIM_CHECK_NULL_1(a)                                                                       \
  do {                                                                                             \
    if ((a) == NULL)                                                                               \
      return HEIM_MAKE_ERR_CTX(HEIM_INVALID_ARGUMENT, #a " is NULL");                              \
  } while (0)

#define HEIM_CHECK_NULL_2(a, b)                                                                    \
  do {                                                                                             \
    HEIM_CHECK_NULL_1(a);                                                                          \
    HEIM_CHECK_NULL_1(b);                                                                          \
  } while (0)

#define HEIM_CHECK_NULL_3(a, b, c)                                                                 \
  do {                                                                                             \
    HEIM_CHECK_NULL_1(a);                                                                          \
    HEIM_CHECK_NULL_1(b);                                                                          \
    HEIM_CHECK_NULL_1(c);                                                                          \
  } while (0)

#define HEIM_CHECK_NULL_4(a, b, c, d)                                                              \
  do {                                                                                             \
    HEIM_CHECK_NULL_1(a);                                                                          \
    HEIM_CHECK_NULL_1(b);                                                                          \
    HEIM_CHECK_NULL_1(c);                                                                          \
    HEIM_CHECK_NULL_1(d);                                                                          \
  } while (0)

#define HEIM_CHECK_NULL_5(a, b, c, d, e)                                                           \
  do {                                                                                             \
    HEIM_CHECK_NULL_1(a);                                                                          \
    HEIM_CHECK_NULL_1(b);                                                                          \
    HEIM_CHECK_NULL_1(c);                                                                          \
    HEIM_CHECK_NULL_1(d);                                                                          \
    HEIM_CHECK_NULL_1(e);                                                                          \
  } while (0)

#define HEIM_CHECK_NULL_6(a, b, c, d, e, f)                                                        \
  do {                                                                                             \
    HEIM_CHECK_NULL_1(a);                                                                          \
    HEIM_CHECK_NULL_1(b);                                                                          \
    HEIM_CHECK_NULL_1(c);                                                                          \
    HEIM_CHECK_NULL_1(d);                                                                          \
    HEIM_CHECK_NULL_1(e);                                                                          \
    HEIM_CHECK_NULL_1(f);                                                                          \
  } while (0)

#define HEIM_GET_CHECK_NULL_MACRO(_1, _2, _3, _4, _5, _6, NAME, ...) NAME
#define HEIM_CHECK_NOT_NULL(...)                                                                   \
  HEIM_GET_CHECK_NULL_MACRO(__VA_ARGS__, HEIM_CHECK_NULL_6, HEIM_CHECK_NULL_5, HEIM_CHECK_NULL_4,  \
                            HEIM_CHECK_NULL_3, HEIM_CHECK_NULL_2, HEIM_CHECK_NULL_1,               \
                            DUMMY)(__VA_ARGS__)

/**
 * @brief Returns a human-readable string literal corresponding to an error code.
 *
 * @param code The error code to stringify.
 * @return Static string literal describing the error code, or "unknown error".
 */
const char *heim_status_code_string(HeimErrorCode code);

/**
 * @brief Formats a status into a human-readable description string.
 *
 * The returned pointer references a thread-local static buffer. Subsequent calls
 * overwrite this buffer.
 *
 * @param status The status struct to format.
 * @return Formatted status string with error code, location, and context.
 */
const char *heim_status_string(HeimStatus status);

/**
 * @brief Prints a formatted status message to standard error (stderr).
 *
 * @param status The status struct to print.
 */
void heim_status_print(HeimStatus status);

/**
 * @brief Prints a formatted status message to the specified output stream.
 *
 * @param stream Target output file stream (e.g. stdout, stderr).
 * @param status The status struct to print.
 */
void heim_status_fprint(FILE *stream, HeimStatus status);

HEIM_END_EXTERN_C

#endif
