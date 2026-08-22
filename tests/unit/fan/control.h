#ifndef ANS_DAEMON_UNIT_FAN_CONTROL_H
#define ANS_DAEMON_UNIT_FAN_CONTROL_H

#include "daemon/types.h"

int unit_run_fan_control(struct ec_device *ec, struct ans_config *cfg,
                             fan_state states[ANS_MAX_FANS]);

#endif
