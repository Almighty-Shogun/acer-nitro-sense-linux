#ifndef ANS_DAEMON_READONLY_COMMAND_H
#define ANS_DAEMON_READONLY_COMMAND_H

#include "daemon/types.h"

bool handle_readonly_command(int client, struct ec_device *ec,
                             const struct ans_config *cfg,
                             const fan_state states[ANS_MAX_FANS],
                             bool auto_mode, const char *preset,
                             bool coolboost_enabled,
                             const daemon_runtime_state *runtime,
                             const char *cmd);

#endif
