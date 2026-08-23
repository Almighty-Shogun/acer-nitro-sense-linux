#ifndef ANS_CONTROL_PROTOCOL_H
#define ANS_CONTROL_PROTOCOL_H

#include <stddef.h>

/**
 * Read one newline-terminated control command.
 *
 * The daemon protocol is line-oriented so each socket request maps to one
 * command dispatch.
 */
int control_read_command(int fd, char* out, size_t out_len);

/**
 * Format and write one control-socket reply.
 *
 * Replies are written through the common descriptor helper so short writes are
 * handled consistently.
 */
int control_reply(int fd, const char* fmt, ...)
__attribute__((format(printf, 2, 3)));

#endif
