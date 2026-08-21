#ifndef ANS_DAEMON_UNIT_STATE_H
#define ANS_DAEMON_UNIT_STATE_H

#include "daemon/types.h"

int unit_run_state_restore(struct ec_device *ec, const struct ans_config *cfg,
                               fan_state states[ANS_MAX_FANS],
                               bool *coolboost_enabled);

#endif
