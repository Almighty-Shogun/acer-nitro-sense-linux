#ifndef ANS_DAEMON_PLATFORM_COMMAND_H
#define ANS_DAEMON_PLATFORM_COMMAND_H

#include "daemon/types.h"

bool handle_platform_command(int client, struct ec_device *ec,
                             const struct ans_config *cfg,
                             fan_state states[ANS_MAX_FANS],
                             bool *auto_mode, char *preset,
                             size_t preset_len, bool *coolboost_enabled,
                             daemon_runtime_state *runtime,
                             const char *cmd);

#endif
