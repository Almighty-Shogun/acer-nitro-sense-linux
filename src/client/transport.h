#ifndef ANS_CLIENT_TRANSPORT_H
#define ANS_CLIENT_TRANSPORT_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Print the daemon status file when available.
 *
 * Status-file output avoids opening the control socket for read-only panel and
 * script usage.
 */
int client_print_status_file(void);

/**
 * Send a daemon command and keep its reply.
 *
 * Callers use this when they need to parse or reformat the daemon response
 * instead of printing it directly.
 */
int client_send_command_capture(const char* command, bool quiet, char* out, size_t out_len);

/**
 * Send a daemon command and print its reply.
 *
 * This is the normal path for CLI commands whose daemon reply is already
 * user-facing.
 */
int client_send_command(const char* command, bool quiet);

/**
 * Format and send a daemon command.
 *
 * Formatting stays in the client layer so daemon command handlers receive a
 * plain protocol line.
 */
int client_send_commandf(bool quiet, const char* format, ...);

#endif
