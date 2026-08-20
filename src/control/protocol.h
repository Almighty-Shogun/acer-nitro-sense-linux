#ifndef ANS_CONTROL_PROTOCOL_H
#define ANS_CONTROL_PROTOCOL_H

#include <stddef.h>

int control_read_command(int fd, char *out, size_t out_len);
int control_reply(int fd, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#endif
