#ifndef ANS_FAN_SAFETY_H
#define ANS_FAN_SAFETY_H

#include "daemon/types.h"

int fan_write_percent_raw(struct ec_device *ec, const struct fan_config *fan,
                          int percent);
void fan_update_safety_state(const struct fan_config *fan, fan_state *state,
                             const char *reason, int requested_percent,
                             int effective_percent);
int fan_safety_adjust_percent(const struct ans_config *cfg,
                              const struct fan_config *fan, fan_state *state,
                              int requested_percent,
                              const char *forced_reason,
                              const char **reason);
int fan_filtered_sensor_temp(const char *fan_id, const char *source,
                             int *current_temp, int *pending_spike_temp,
                             int sensor_temp, bool trusted_baseline,
                             int critical_temperature_c);

#endif
