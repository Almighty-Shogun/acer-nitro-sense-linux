#ifndef ANS_DAEMON_EC_COMMAND_H
#define ANS_DAEMON_EC_COMMAND_H

#include "daemon/types.h"

bool handle_ec_command(int client, struct ec_device *ec, const char *cmd);

#endif
