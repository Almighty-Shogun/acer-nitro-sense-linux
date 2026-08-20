#ifndef ANS_DAEMON_SELFTEST_PLATFORM_COMMANDS_H
#define ANS_DAEMON_SELFTEST_PLATFORM_COMMANDS_H

#include "ans.h"
#include "daemon/types.h"

int selftest_run_platform_commands(struct ec_device *ec,
                                   const struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS]);

#endif
