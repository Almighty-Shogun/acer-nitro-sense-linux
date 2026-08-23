#include "util/fd.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

/**
 * Read from a file descriptor while tolerating signal interruption.
 *
 * EINTR is not a real read failure for the daemon's sockets and sysfs files, so
 * this wrapper retries until the kernel returns data, EOF, or a real error.
 */
ssize_t fd_read_retry(const int fd, void* buf, const size_t len)
{
    ssize_t n;

    do
    {
        n = read(fd, buf, len);
    }
    while (n < 0 && errno == EINTR);

    return n;
}

/**
 * Write an entire buffer to a file descriptor.
 *
 * Short writes are normal for descriptors. The caller gets success only after
 * every byte was accepted, and a zero-length write is treated as an I/O failure.
 */
int fd_write_all(const int fd, const void* buf, const size_t len)
{
    size_t off = 0;

    const char* bytes = buf;

    while (off < len)
    {
        const ssize_t n = write(fd, bytes + off, len - off);

        if (n < 0)
        {
            if (errno == EINTR) continue;

            return -1;
        }

        if (n == 0)
        {
            errno = EIO;

            return -1;
        }

        off += (size_t)n;
    }

    return 0;
}

/**
 * Write a NUL-terminated string to a file descriptor.
 *
 * This keeps reply-writing code focused on protocol text rather than repeated
 * length calculation.
 */
int fd_write_string(const int fd, const char* text)
{
    return fd_write_all(fd, text, strlen(text));
}
