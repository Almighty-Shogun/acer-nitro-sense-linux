#ifndef ANS_UTIL_PROCESS_H
#define ANS_UTIL_PROCESS_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

/**
 * Start a child process and capture its output stream.
 *
 * Doctor and service helpers use this instead of shelling out through a single
 * formatted command string.
 */
FILE *process_open_output(const char *file, const char *const argv[],
                          bool stderr_to_stdout, pid_t *pid_out);
/**
 * Start a child process and capture stdout.
 *
 * This is the common case for commands whose stderr should remain separate.
 */
FILE *process_open_stdout(const char *file, const char *const argv[],
                          pid_t *pid_out);
/**
 * Close a captured process stream and wait for the child.
 *
 * The returned status reflects whether the child completed successfully.
 */
int process_close_stdout(FILE *stream, pid_t pid);

#endif
