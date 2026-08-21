#ifndef ANS_UTIL_PROCESS_H
#define ANS_UTIL_PROCESS_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

FILE *process_open_output(const char *file, const char *const argv[],
                          bool stderr_to_stdout, pid_t *pid_out);
FILE *process_open_stdout(const char *file, const char *const argv[],
                          pid_t *pid_out);
int process_close_stdout(FILE *stream, pid_t pid);

#endif
