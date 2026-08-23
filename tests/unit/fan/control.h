#ifndef ANS_DAEMON_UNIT_FAN_CONTROL_H
#define ANS_DAEMON_UNIT_FAN_CONTROL_H

#include "daemon/types.h"

/**
 * Run all fan control unit tests.
 *
 * The grouped runner exercises fan safety, automatic control, EC temperature,
 * and resume behavior with a fake EC device.
 */
int unit_run_fan_control(struct ec_device* ec, struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

#endif
