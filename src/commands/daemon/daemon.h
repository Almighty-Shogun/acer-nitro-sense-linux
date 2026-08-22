#ifndef ANS_DAEMON_COMMAND_H
#define ANS_DAEMON_COMMAND_H

#include "daemon/types.h"

/* Exposed for the socket loop and unit tests. Individual command handlers stay
 * inside src/commands/daemon. */
void execute_command(int client, struct ec_device *ec, const struct ans_config *cfg,
                     fan_state states[ANS_MAX_FANS], bool *auto_mode,
                     char *preset, size_t preset_len,
                     bool *coolboost_enabled,
                     daemon_runtime_state *runtime,
                     const char *cmd, bool can_control);
void handle_client(int client, struct ec_device *ec, const struct ans_config *cfg,
                   fan_state states[ANS_MAX_FANS], bool *auto_mode,
                   char *preset, size_t preset_len,
                   bool *coolboost_enabled,
                   daemon_runtime_state *runtime);

#endif
