#include "control/protocol.h"

#include "util/fd.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

/**
 * Read control command.
 *
 * The control socket is the privilege boundary for non-root clients. Keeping
 * this logic small makes permission failures explicit and state-changing
 * commands auditable.
 */
int control_read_command(const int fd, char* out, const size_t out_len)
{
    size_t used = 0;

    if (out_len == 0)
    {
        errno = EINVAL;

        return -1;
    }

    while (used + 1 < out_len)
    {
        char ch;
        const ssize_t n = fd_read_retry(fd, &ch, 1);

        if (n < 0)
            return -1;

        if (n == 0 || ch == '\n') break;

        out[used++] = ch;
    }

    out[used] = '\0';

    if (used + 1 == out_len)
    {
        errno = E2BIG;

        return -1;
    }

    return used > 0 ? 0 : -1;
}

/**
 * Write control.
 *
 * The control socket is the privilege boundary for non-root clients. Keeping
 * this logic small makes permission failures explicit and state-changing
 * commands auditable.
 */
int control_reply(const int fd, const char* fmt, ...)
{
    va_list ap;

    char buf[4096];

    va_start(ap, fmt);
    const int written = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (written < 0)
        return -1;

    if ((size_t)written >= sizeof(buf))
    {
        errno = E2BIG;

        return -1;
    }

    return fd_write_all(fd, buf, (size_t)written);
}
