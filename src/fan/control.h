#ifndef ANS_FAN_CONTROL_H
#define ANS_FAN_CONTROL_H

#include "daemon/types.h"

int set_fan_percent(struct ec_device *ec, const struct ans_config *cfg,
                    const struct fan_config *fan, fan_state *state,
                    int requested_percent, const char *forced_reason);
const char *global_safety_reason(const struct ans_config *cfg,
                                 const fan_state states[ANS_MAX_FANS]);
void apply_init_writes(struct ec_device *ec, const struct ans_config *cfg);
void apply_reset_writes(struct ec_device *ec, const struct ans_config *cfg);
int auto_ramped_percent(const struct ans_config *cfg, const fan_state *state,
                        int target_percent, const char *forced_reason);
void update_fan_states(struct ec_device *ec, const struct ans_config *cfg,
                       fan_state states[ANS_MAX_FANS], bool auto_mode,
                       const char *preset);
void seed_last_temperatures(const struct ans_config *cfg,
                            fan_state states[ANS_MAX_FANS]);
int set_one(struct ec_device *ec, const struct ans_config *cfg,
            fan_state states[ANS_MAX_FANS], const char *id, int percent);
bool apply_preset(struct ec_device *ec, const struct ans_config *cfg,
                  fan_state states[ANS_MAX_FANS], const char *id);
void apply_current_control_state(struct ec_device *ec, const struct ans_config *cfg,
                                 fan_state states[ANS_MAX_FANS]);

#endif
