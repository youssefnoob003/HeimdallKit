#ifndef HEIM_INTERNAL_IO_H
#define HEIM_INTERNAL_IO_H

#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Transactional file output handle using atomic temporary files.
 */
typedef struct {
  FILE *fp;             /**< Open file pointer to temporary staging file */
  char *temporary_path; /**< Dynamically allocated path to temporary file */
} HeimOutputFile;

/**
 * @brief Opens a staging file for atomic file writes.
 *
 * On POSIX systems, creates a temporary file using mkstemp. On Windows, creates a temporary file
 * beside the destination using an exclusive file open.
 *
 * @param filepath Destination filesystem path.
 * @param[out] output Output structure to initialize.
 * @return True on success, false on failure.
 */
bool heim_io_open_output(const char *filepath, HeimOutputFile *output);

/**
 * @brief Closes the transactional output file and atomically renames it to target path if
 * successful.
 *
 * If ok is false, cleans up and removes temporary staging file.
 *
 * @param output Output structure containing file handle and temporary path.
 * @param filepath Target destination filesystem path.
 * @param ok Flag indicating whether all file write operations completed successfully.
 * @return True if file was cleanly closed and committed, false otherwise.
 */
bool heim_io_close_output(HeimOutputFile *output, const char *filepath, bool ok);

#endif
