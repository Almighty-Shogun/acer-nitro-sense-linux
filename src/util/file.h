#ifndef ANS_UTIL_FILE_H
#define ANS_UTIL_FILE_H

#include <stddef.h>

int mkdir_p(const char *path);
char *read_text_file(const char *path, size_t limit);
int write_text_file_atomic(const char *path, const char *content);

#endif
