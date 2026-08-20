#ifndef ANS_CLIENT_TRANSPORT_H
#define ANS_CLIENT_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>

int client_print_status_file(void);
int client_send_command_capture(const char *command, bool quiet, char *out,
                                size_t out_len);
int client_send_command(const char *command, bool quiet);
int client_send_commandf(bool quiet, const char *format, ...);

#endif
