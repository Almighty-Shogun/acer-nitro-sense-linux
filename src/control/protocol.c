#include "control/protocol.h"

#include "util/fd.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

int control_read_command(const int fd, char *out, const size_t out_len)
{
    size_t used = 0;

    if (out_len == 0) {
        errno = EINVAL;
        return -1;
    }

    while (used + 1 < out_len) {
        char ch;
        const ssize_t n = fd_read_retry(fd, &ch, 1);

        if (n < 0)
            return -1;
        if (n == 0)
            break;
        if (ch == '\n')
            break;

        out[used++] = ch;
    }

    out[used] = '\0';

    if (used + 1 == out_len) {
        errno = E2BIG;
        return -1;
    }

    return used > 0 ? 0 : -1;
}

int control_reply(const int fd, const char *fmt, ...)
{
    char buf[4096];
    va_list ap;
    int written;

    va_start(ap, fmt);
    written = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (written < 0)
        return -1;

    if ((size_t)written >= sizeof(buf)) {
        errno = E2BIG;
        return -1;
    }

    return fd_write_all(fd, buf, (size_t)written);
}
