#ifndef ANS_SELFTEST_FIXTURE_H
#define ANS_SELFTEST_FIXTURE_H

#include "ans.h"
#include "daemon/types.h"

void init_self_test_config(struct ans_config *cfg);
void reset_self_test_states(const struct ans_config *cfg,
                            fan_state states[ANS_MAX_FANS]);

#endif
