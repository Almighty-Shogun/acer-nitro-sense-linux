#ifndef ANS_DAEMON_EC_COMMAND_H
#define ANS_DAEMON_EC_COMMAND_H

#include "daemon/types.h"

/**
 * Dispatch raw EC debug commands.
 *
 * These commands are intended for diagnostics and model bring-up after the
 * daemon registry has already enforced control permissions.
 */
bool handle_ec_command(int client, struct ec_device* ec, const char* cmd);

#endif
