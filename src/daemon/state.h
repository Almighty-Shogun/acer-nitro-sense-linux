#ifndef ANS_DAEMON_STATE_H
#define ANS_DAEMON_STATE_H

#include "daemon/types.h"

void write_control_state(const struct ans_config *cfg,
                         const fan_state states[ANS_MAX_FANS],
                         bool auto_mode, const char *preset,
                         bool coolboost_enabled,
                         const daemon_runtime_state *runtime);
bool restore_control_state_from_json(struct ec_device *ec, const struct ans_config *cfg,
                                     fan_state states[ANS_MAX_FANS],
                                     bool *auto_mode, char *preset,
                                     size_t preset_len,
                                     bool *coolboost_enabled,
                                     daemon_runtime_state *runtime,
                                     const char *json);
bool restore_control_state(struct ec_device *ec, const struct ans_config *cfg,
                           fan_state states[ANS_MAX_FANS], bool *auto_mode,
                           char *preset, size_t preset_len,
                           bool *coolboost_enabled,
                           daemon_runtime_state *runtime);

#endif
