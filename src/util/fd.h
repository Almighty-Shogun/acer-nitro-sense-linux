#ifndef ANS_UTIL_FD_H
#define ANS_UTIL_FD_H

#include <stddef.h>
#include <sys/types.h>

ssize_t fd_read_retry(int fd, void *buf, size_t len);
int fd_write_all(int fd, const void *buf, size_t len);
int fd_write_string(int fd, const char *text);

#endif
