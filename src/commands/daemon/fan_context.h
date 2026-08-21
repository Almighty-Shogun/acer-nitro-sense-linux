#ifndef ANS_DAEMON_FAN_CONTEXT_H
#define ANS_DAEMON_FAN_CONTEXT_H

#include "daemon/types.h"

typedef struct {
    int client;
    struct ec_device *ec;
    const struct ans_config *cfg;
    fan_state *states;
    bool *auto_mode;
    char *preset;
    size_t preset_len;
    bool *coolboost_enabled;
    const daemon_runtime_state *runtime;
} fan_command_context;

bool fan_command_prepare_daemon_control(const fan_command_context *ctx);
void fan_command_set_control_mode(const fan_command_context *ctx,
                                  bool auto_enabled,
                                  const char *preset_name);
void fan_command_write_state(const fan_command_context *ctx);

#endif
