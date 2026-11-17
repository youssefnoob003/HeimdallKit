#include "heim/io_internal.h"

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool heim_io_open_output(const char *filepath, HeimOutputFile *output) {
  if (!filepath || !output)
    return false;

  output->fp = NULL;
  output->temporary_path = NULL;

  size_t path_length = strlen(filepath);
  if (path_length > SIZE_MAX - 12)
    return false;

  output->temporary_path = malloc(path_length + 12);
  if (!output->temporary_path)
    return false;
  int written = snprintf(output->temporary_path, path_length + 12, "%s.tmp.XXXXXX", filepath);
  if (written < 0 || (size_t)written >= path_length + 12) {
    free(output->temporary_path);
    output->temporary_path = NULL;
    return false;
  }

#if defined(_WIN32)
  if (_mktemp_s(output->temporary_path, path_length + 12) != 0) {
    free(output->temporary_path);
    output->temporary_path = NULL;
    return false;
  }

  int fd = -1;
  if (_sopen_s(&fd, output->temporary_path, _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _SH_DENYRW,
               _S_IREAD | _S_IWRITE) != 0) {
    free(output->temporary_path);
    output->temporary_path = NULL;
    return false;
  }
  output->fp = _fdopen(fd, "wb");
#else
  int fd = mkstemp(output->temporary_path);
  output->fp = NULL;
#endif

#if defined(_WIN32)
  if (!output->fp) {
    _close(fd);
    remove(output->temporary_path);
    free(output->temporary_path);
    output->temporary_path = NULL;
    return false;
  }
#else
  if (fd < 0) {
    free(output->temporary_path);
    output->temporary_path = NULL;
    return false;
  }
  output->fp = fdopen(fd, "wb");
  if (!output->fp) {
    close(fd);
    remove(output->temporary_path);
    free(output->temporary_path);
    output->temporary_path = NULL;
    return false;
  }
#endif
  return true;
}

bool heim_io_close_output(HeimOutputFile *output, const char *filepath, bool ok) {
  if (!output || !output->fp || !filepath)
    return false;
  if (fclose(output->fp) != 0)
    ok = false;

#if defined(_WIN32)
  if (ok && !MoveFileExA(output->temporary_path, filepath,
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    ok = false;
  if (!ok)
    remove(output->temporary_path);
  free(output->temporary_path);
  output->temporary_path = NULL;
#else
  if (ok && rename(output->temporary_path, filepath) != 0)
    ok = false;
  if (!ok)
    remove(output->temporary_path);
  free(output->temporary_path);
  output->temporary_path = NULL;
#endif
  output->fp = NULL;
  return ok;
}
