#ifndef ANS_DAEMON_PLATFORM_HANDLERS_H
#define ANS_DAEMON_PLATFORM_HANDLERS_H

#include "daemon/types.h"

bool handle_coolboost_command(int client, struct ec_device *ec,
                              const struct ans_config *cfg,
                              fan_state states[ANS_MAX_FANS], bool auto_mode,
                              const char *preset, bool *coolboost_enabled,
                              const daemon_runtime_state *runtime,
                              const char *cmd);
bool handle_fan_mode_command(int client, struct ec_device *ec,
                             const struct ans_config *cfg,
                             fan_state states[ANS_MAX_FANS], bool *auto_mode,
                             char *preset, size_t preset_len,
                             bool *coolboost_enabled,
                             const daemon_runtime_state *runtime,
                             const char *cmd);
bool handle_profile_command(int client, struct ec_device *ec,
                            const struct ans_config *cfg, const char *cmd);
bool handle_power_source_command(int client, struct ec_device *ec,
                                 const struct ans_config *cfg,
                                 const fan_state states[ANS_MAX_FANS],
                                 bool auto_mode, const char *preset,
                                 bool coolboost_enabled,
                                 daemon_runtime_state *runtime,
                                 const char *cmd);
bool handle_gpu_temp_command(int client, const char *cmd);
bool handle_keyboard_backlight_command(int client, struct ec_device *ec,
                                       const struct ans_config *cfg,
                                       const fan_state states[ANS_MAX_FANS],
                                       bool auto_mode, const char *preset,
                                       bool coolboost_enabled,
                                       daemon_runtime_state *runtime,
                                       const char *cmd);

#endif
