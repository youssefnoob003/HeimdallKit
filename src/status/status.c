#include "heim/status.h"

#include <stdio.h>

const char *heim_status_code_string(HeimErrorCode code) {
  switch (code) {
  case HEIM_SUCCESS:
    return "ok";
  case HEIM_INVALID_ARGUMENT:
    return "invalid argument";
  case HEIM_OUT_OF_RANGE:
    return "out of range";
  case HEIM_INCOMPATIBLE:
    return "incompatible";
  case HEIM_ALLOCATION_FAILED:
    return "allocation failed";
  case HEIM_IO_ERROR:
    return "I/O error";
  case HEIM_FORMAT_ERROR:
    return "format error";
  case HEIM_OVERFLOW:
    return "overflow";
  default:
    return "unknown status";
  }
}

const char *heim_status_string(HeimStatus status) {
  static _Thread_local char buffer[512];

  if (status.code == HEIM_SUCCESS) {
    (void)snprintf(buffer, sizeof(buffer), "[HEIM OK]");
  } else if (status.file && status.context) {
    (void)snprintf(buffer, sizeof(buffer), "[HEIM ERROR: %s at %s:%d (%s)]",
                   heim_status_code_string(status.code), status.file, status.line, status.context);
  } else if (status.file) {
    (void)snprintf(buffer, sizeof(buffer), "[HEIM ERROR: %s at %s:%d]",
                   heim_status_code_string(status.code), status.file, status.line);
  } else if (status.context) {
    (void)snprintf(buffer, sizeof(buffer), "[HEIM ERROR: %s (%s)]",
                   heim_status_code_string(status.code), status.context);
  } else {
    (void)snprintf(buffer, sizeof(buffer), "[HEIM ERROR: %s]",
                   heim_status_code_string(status.code));
  }

  buffer[sizeof(buffer) - 1] = '\0';
  return buffer;
}

void heim_status_fprint(FILE *stream, HeimStatus status) {
  if (!stream)
    stream = stderr;

  fprintf(stream, "%s\n", heim_status_string(status));
}

void heim_status_print(HeimStatus status) {
  heim_status_fprint(stderr, status);
}
