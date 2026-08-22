#ifndef ANS_CONFIG_SAFETY_TYPES_H
#define ANS_CONFIG_SAFETY_TYPES_H

#include <stdbool.h>

struct safety_config {
    int min_speed_percent;
    int min_speed_temperature_c;
    int critical_speed_percent;
    bool critical_full_speed;
    int critical_step_percent;
    int critical_consecutive_samples;
    int critical_release_temperature_c;
    int auto_ramp_up_percent;
    int auto_ramp_bypass_temperature_c;
    int missing_temperature_speed_percent;
    int max_ec_read_failures;
    int max_ec_write_failures;
};

#endif
