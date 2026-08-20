#include "util/format.h"

#include <stdarg.h>
#include <stdio.h>

void text_buffer_init(text_buffer *buf, char *data, const size_t cap)
{
    buf->data = data;
    buf->len = 0;
    buf->cap = cap;
    buf->truncated = false;

    if (cap > 0)
        data[0] = '\0';
}

int text_buffer_append(text_buffer *buf, const char *fmt, ...)
{
    va_list ap;
    int written;

    if (buf->len >= buf->cap) {
        buf->truncated = true;
        return -1;
    }

    va_start(ap, fmt);
    written = vsnprintf(buf->data + buf->len, buf->cap - buf->len, fmt, ap);
    va_end(ap);

    if (written < 0) {
        buf->truncated = true;
        return -1;
    }

    if ((size_t)written >= buf->cap - buf->len) {
        buf->len = buf->cap - 1;
        buf->truncated = true;
        return -1;
    }

    buf->len += (size_t)written;
    return 0;
}

bool text_buffer_ok(const text_buffer *buf)
{
    return !buf->truncated;
}
