#ifndef ANS_DAEMON_CONTROL_COMMAND_H
#define ANS_DAEMON_CONTROL_COMMAND_H

#include "daemon/types.h"

/**
 * Dispatch daemon lifecycle commands.
 *
 * Resume reapplies EC state after suspend, while stop requests a clean daemon
 * shutdown and firmware reset.
 */
bool handle_daemon_control_command(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    const char* cmd
);

#endif
