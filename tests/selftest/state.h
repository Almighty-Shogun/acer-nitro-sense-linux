#ifndef ANS_DAEMON_SELFTEST_STATE_H
#define ANS_DAEMON_SELFTEST_STATE_H

#include "ans.h"
#include "daemon/types.h"

int selftest_run_state_restore(struct ec_device *ec, const struct ans_config *cfg,
                               fan_state states[ANS_MAX_FANS],
                               bool *coolboost_enabled);

#endif
