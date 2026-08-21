#ifndef ANS_UNIT_FIXTURE_H
#define ANS_UNIT_FIXTURE_H

#include "daemon/types.h"

void init_unit_test_config(struct ans_config *cfg);
void reset_unit_test_states(const struct ans_config *cfg,
                            fan_state states[ANS_MAX_FANS]);

#endif
