#ifndef ANS_FAN_OBSERVATION_H
#define ANS_FAN_OBSERVATION_H

#include "daemon/types.h"

/**
 * Refresh all fan sensor observations.
 *
 * Observation refresh updates RPM, temperature, filtered-temperature, and EC
 * failure counters before fan control decisions are made.
 */
const char* refresh_fan_observations(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

#endif
