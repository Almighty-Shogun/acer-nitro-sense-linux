#ifndef ANS_DAEMON_FEATURE_STATUS_H
#define ANS_DAEMON_FEATURE_STATUS_H

#include "daemon/types.h"

void reply_coolboost_status(int client, const struct ans_config *cfg,
                            bool coolboost_enabled);
void reply_fan_mode_status(int client, struct ec_device *ec,
                           const struct ans_config *cfg);
void reply_profile_status(int client, struct ec_device *ec,
                          const struct ans_config *cfg);
void reply_power_source_status(int client, struct ec_device *ec,
                               const struct ans_config *cfg,
                               const daemon_runtime_state *runtime);
void reply_gpu_temp_status(int client, struct ec_device *ec,
                           const struct ans_config *cfg);
void reply_keyboard_backlight_status(int client, struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     const daemon_runtime_state *runtime);

#endif
