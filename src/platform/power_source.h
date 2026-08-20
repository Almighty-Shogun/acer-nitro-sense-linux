#ifndef ANS_POWER_SOURCE_H
#define ANS_POWER_SOURCE_H

#include "ans.h"

enum power_source_state {
    POWER_SOURCE_UNKNOWN = 0,
    POWER_SOURCE_AC,
    POWER_SOURCE_BATTERY,
};

const char *power_source_name(enum power_source_state source);
enum power_source_state read_power_source(void);
const char *power_source_profile_for(const struct ans_config *cfg,
                                     enum power_source_state source);
bool power_source_profile_policy_available(const struct ans_config *cfg);
bool apply_power_source_profile(struct ec_device *ec, const struct ans_config *cfg,
                                enum power_source_state source);

#endif
