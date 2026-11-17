#include "heim/status.h"

#include <assert.h>

static HeimStatus check_i64_add_overflow(int64_t a, int64_t b) {
  HEIM_CHECK_ADD_OVERFLOW_I64(a, b);
  return HEIM_OK;
}

static HeimStatus check_size_add_overflow(size_t a, size_t b) {
  HEIM_CHECK_ADD_OVERFLOW_SIZE(a, b);
  return HEIM_OK;
}

int main(void) {
  assert(check_i64_add_overflow(INT64_MAX, 1).code == HEIM_OVERFLOW);
  assert(check_i64_add_overflow(INT64_MIN, -1).code == HEIM_OVERFLOW);
  assert(check_i64_add_overflow(0, 1).code == HEIM_SUCCESS);
  assert(check_i64_add_overflow(INT64_MIN, 1).code == HEIM_SUCCESS);
  assert(check_i64_add_overflow(INT64_MAX, -1).code == HEIM_SUCCESS);

  assert(check_size_add_overflow(SIZE_MAX, 1).code == HEIM_OVERFLOW);
  assert(check_size_add_overflow(0, 1).code == HEIM_SUCCESS);
  return 0;
}
