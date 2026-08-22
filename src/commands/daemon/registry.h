#ifndef ANS_DAEMON_COMMAND_REGISTRY_H
#define ANS_DAEMON_COMMAND_REGISTRY_H

#include "daemon/types.h"

/**
 * Runtime context shared by the daemon command registry.
 *
 * The registry receives one context object so top-level routing can pass
 * command state to feature handlers without duplicating long parameter lists.
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
    daemon_runtime_state* runtime;
    const char* cmd;
    bool can_control;
} daemon_command_context;

/**
 * Dispatch one parsed daemon command.
 *
 * The registry owns top-level command routing so feature handlers do not need
 * to probe commands that belong to other features.
 */
void dispatch_daemon_command(const daemon_command_context* ctx);

#endif
