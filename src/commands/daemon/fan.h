#ifndef ANS_DAEMON_FAN_COMMAND_H
#define ANS_DAEMON_FAN_COMMAND_H

#include "daemon/types.h"

/**
 * Dispatch daemon fan-control commands.
 *
 * Manual speed, preset, daemon-auto, and firmware-auto commands all update the
 * same persisted control state.
 */
bool handle_fan_control_command(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    const daemon_runtime_state* runtime,
    const char* cmd
);

#endif
