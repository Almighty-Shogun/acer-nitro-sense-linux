#ifndef ANS_FAN_INTERNAL_H
#define ANS_FAN_INTERNAL_H

#include "daemon/types.h"

/**
 * Write a fan percentage without safety adjustment.
 *
 * Public fan writes go through safety first; this helper performs only the EC
 * range conversion and register write.
 */
int fan_write_percent_raw(struct ec_device* ec, const struct fan_config* fan, int percent);

/**
 * Record the latest safety decision for a fan.
 *
 * Status output reports requested and effective percentages from this state so
 * users can see when safety changed their request.
 */
void fan_update_safety_state(
    const struct fan_config* fan,
    fan_state* state,
    const char* reason,
    int requested_percent,
    int effective_percent
);

/**
 * Apply fan safety policy to a requested percentage.
 *
 * The returned value is the effective speed after critical-temperature,
 * missing-temperature, and EC-failure rules are applied.
 */
int fan_safety_adjust_percent(
    const struct ans_config* cfg,
    const struct fan_config* fan,
    const fan_state* state,
    int requested_percent,
    const char* forced_reason,
    const char** reason
);

#endif
