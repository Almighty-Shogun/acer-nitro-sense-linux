#ifndef ANS_UTIL_FORMAT_H
#define ANS_UTIL_FORMAT_H

#include <stdbool.h>
#include <stddef.h>

typedef struct text_buffer {
    char *data;
    size_t len;
    size_t cap;
    bool truncated;
} text_buffer;

void text_buffer_init(text_buffer *buf, char *data, size_t cap);
int text_buffer_append(text_buffer *buf, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
bool text_buffer_ok(const text_buffer *buf);

#endif
