#ifndef ANS_DAEMON_SELFTEST_FAN_CASES_H
#define ANS_DAEMON_SELFTEST_FAN_CASES_H

#include "daemon/types.h"

int selftest_run_fan_safety_clamps(struct ec_device *ec, struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS]);
int selftest_run_fan_ec_write_failures(struct ec_device *ec,
                                       struct ans_config *cfg,
                                       fan_state states[ANS_MAX_FANS]);
int selftest_run_fan_temperature_safety(struct ec_device *ec,
                                        struct ans_config *cfg,
                                        fan_state states[ANS_MAX_FANS]);
int selftest_run_fan_ec_temperature_register(struct ec_device *ec,
                                             struct ans_config *cfg,
                                             fan_state states[ANS_MAX_FANS]);
int selftest_run_fan_auto_ramp(struct ec_device *ec, struct ans_config *cfg,
                               fan_state states[ANS_MAX_FANS]);
int selftest_run_fan_preset_resume_status(struct ec_device *ec,
                                          struct ans_config *cfg,
                                          fan_state states[ANS_MAX_FANS]);

#endif
