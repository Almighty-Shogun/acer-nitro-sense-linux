#ifndef ANS_UTIL_PROCESS_H
#define ANS_UTIL_PROCESS_H

#include <stdio.h>
#include <sys/types.h>

FILE *process_open_stdout(const char *file, char *const argv[], pid_t *pid_out);
int process_close_stdout(FILE *stream, pid_t pid);

#endif
