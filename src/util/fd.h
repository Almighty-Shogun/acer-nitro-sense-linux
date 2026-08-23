#ifndef ANS_UTIL_FD_H
#define ANS_UTIL_FD_H

#include <stddef.h>
#include <sys/types.h>

/**
 * Read from a descriptor while retrying interrupted syscalls.
 *
 * Signal interruptions should not make control-socket or EC file reads fail
 * spuriously.
 */
ssize_t fd_read_retry(int fd, void* buf, size_t len);

/**
 * Write an entire buffer to a descriptor.
 *
 * Short writes and interrupted syscalls are handled until the full buffer is
 * written or a real error occurs.
 */
int fd_write_all(int fd, const void* buf, size_t len);

/**
 * Write a NUL-terminated string to a descriptor.
 *
 * Control replies use this wrapper so string length calculation stays out of
 * protocol code.
 */
int fd_write_string(int fd, const char* text);

#endif
