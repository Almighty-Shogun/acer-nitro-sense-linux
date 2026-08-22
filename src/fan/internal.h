#ifndef ANS_FAN_INTERNAL_H
#define ANS_FAN_INTERNAL_H

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

#endif
