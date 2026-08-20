#ifndef ANS_FAN_OBSERVATION_H
#define ANS_FAN_OBSERVATION_H

#include "daemon/types.h"

const char *refresh_fan_observations(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     fan_state states[ANS_MAX_FANS]);

#endif
