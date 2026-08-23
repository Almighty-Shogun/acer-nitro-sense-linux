#ifndef ANS_DAEMON_FAN_CONTEXT_H
#define ANS_DAEMON_FAN_CONTEXT_H

#include "daemon/types.h"

/**
 * Runtime context shared by daemon fan-control handlers.
 *
 * Fan commands all touch the same EC device, model profile, persisted preset,
 * and runtime state. Passing one context keeps the handlers small without
 * hiding ownership.
 */
typedef struct
{
    int client;
    struct ec_device* ec;
    const struct ans_config* cfg;
    fan_state* states;
    bool* auto_mode;
    char* preset;
    size_t preset_len;
    bool* coolboost_enabled;
    const daemon_runtime_state* runtime;
} fan_command_context;

/**
 * Prepare fan commands for daemon-owned control.
 *
 * Manual and preset fan commands must disable firmware fan mode before writing
 * explicit speed values.
 */
bool fan_command_prepare_daemon_control(const fan_command_context* ctx);

/**
 * Update the current daemon fan-control mode.
 *
 * The mode is stored through pointers in the shared context so the command
 * handler and the polling loop observe the same state.
 */
void fan_command_set_control_mode(const fan_command_context* ctx, bool auto_enabled, const char* preset_name);

/**
 * Persist the current fan-control state.
 *
 * Persistence lets the daemon restore the last selected mode after service
 * restart without guessing from EC register values.
 */
void fan_command_write_state(const fan_command_context* ctx);

#endif
