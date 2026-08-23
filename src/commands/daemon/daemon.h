#ifndef ANS_DAEMON_COMMAND_H
#define ANS_DAEMON_COMMAND_H

#include "daemon/types.h"

/**
 * Execute one daemon control-socket command.
 *
 * The command is routed through the registry with the caller's permission
 * state, letting read-only commands and mutating commands share one entry
 * point.
 */
void execute_command(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd,
    bool can_control
);

/**
 * Read and handle one connected control-socket client.
 *
 * The handler reads a single request, dispatches it, writes the reply, and
 * leaves connection lifetime management to the socket loop.
 */
void handle_client(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime
);

#endif
