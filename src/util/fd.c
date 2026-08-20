#include "util/fd.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

ssize_t fd_read_retry(const int fd, void *buf, const size_t len)
{
    ssize_t n;

    do {
        n = read(fd, buf, len);
    } while (n < 0 && errno == EINTR);

    return n;
}

int fd_write_all(const int fd, const void *buf, const size_t len)
{
    const char *bytes = buf;
    size_t off = 0;

    while (off < len) {
        const ssize_t n = write(fd, bytes + off, len - off);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0) {
            errno = EIO;
            return -1;
        }

        off += (size_t)n;
    }

    return 0;
}

int fd_write_string(const int fd, const char *text)
{
    return fd_write_all(fd, text, strlen(text));
}
