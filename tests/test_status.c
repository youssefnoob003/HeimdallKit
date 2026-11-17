#include "heim/heim.h"

#include <assert.h>
#include <math.h>
#include <string.h>

static void test_status_contract(void) {
  HeimSig *sig = NULL;

  assert(heim_status_string(HEIM_OK) != NULL);
  assert(heim_status_code_string(HEIM_INCOMPATIBLE) != NULL);
  assert(heim_status_code_string(HEIM_ALLOCATION_FAILED) != NULL);
  assert(heim_status_code_string((HeimErrorCode)999) != NULL);

  HeimStatus s = heim_sig_alloc(0, 1.0, 1, NULL);
  assert(s.code == HEIM_INVALID_ARGUMENT);
  assert(s.file != NULL);
  assert(s.line > 0);
  assert(s.context != NULL && strstr(s.context, "out_sig") != NULL);

  assert(heim_sig_alloc(0, 0.0, 1, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_alloc(INT64_MAX, 1.0, 2, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(HEIM_IS_OK(heim_sig_alloc(0, 1.0, 1, &sig)));
  assert(sig != NULL);
  heim_sig_free(sig);
}

static void test_consistent_validation(void) {
  HeimSig *sig = NULL;
  double power = 0.0;

  assert(heim_sig_gen_exp(NAN, 0.0, 0.0, 0, 1, 1.0, &sig).code == HEIM_INVALID_ARGUMENT);
  assert(heim_sig_an_power_range(NULL, 0, 1, &power).code == HEIM_INVALID_ARGUMENT);
  assert(HEIM_IS_OK(heim_sig_alloc(0, 1.0, 1, &sig)));
  assert(heim_sig_an_power_range(sig, 1, 0, &power).code == HEIM_INVALID_ARGUMENT);
  heim_sig_free(sig);
}

static void test_status_macros_and_printing(void) {
  HeimStatus ok = HEIM_OK;
  assert(HEIM_IS_OK(ok));
  assert(!HEIM_IS_ERR(ok));

  HeimStatus err = HEIM_MAKE_ERR(HEIM_OUT_OF_RANGE);
  assert(HEIM_IS_ERR(err));
  assert(err.code == HEIM_OUT_OF_RANGE);
  assert(err.file != NULL);
  assert(err.line > 0);
  assert(err.context == NULL);

  HeimStatus err_ctx = HEIM_MAKE_ERR_CTX(HEIM_INCOMPATIBLE, "sampling rate mismatch");
  assert(err_ctx.code == HEIM_INCOMPATIBLE);
  assert(strcmp(err_ctx.context, "sampling rate mismatch") == 0);

  const char *formatted = heim_status_string(err_ctx);
  assert(strstr(formatted, "incompatible") != NULL);
  assert(strstr(formatted, "sampling rate mismatch") != NULL);

  /* Test fprint */
  FILE *tmp = tmpfile();
  if (tmp) {
    heim_status_fprint(tmp, ok);
    heim_status_fprint(tmp, err_ctx);
    rewind(tmp);
    char buf[256];
    assert(fgets(buf, sizeof(buf), tmp) != NULL);
    assert(strstr(buf, "HEIM OK") != NULL);
    assert(fgets(buf, sizeof(buf), tmp) != NULL);
    assert(strstr(buf, "incompatible") != NULL);
    assert(strstr(buf, "sampling rate mismatch") != NULL);
    fclose(tmp);
  }
}

int main(void) {
  test_status_contract();
  test_consistent_validation();
  test_status_macros_and_printing();
  return 0;
}
