#ifndef ANS_UTIL_FILE_H
#define ANS_UTIL_FILE_H

#include <stddef.h>

/**
 * Create a directory path recursively.
 *
 * Missing parent directories are created with normal daemon state-file
 * permissions.
 */
int mkdir_p(const char* path);

/**
 * Read a text file up to a fixed byte limit.
 *
 * The returned buffer is heap-allocated and owned by the caller.
 */
char* read_text_file(const char* path, size_t limit);

/**
 * Atomically replace a text file.
 *
 * State writes go through a temporary file and rename so readers never see a
 * partially written payload.
 */
int write_text_file_atomic(const char* path, const char* content);

#endif
