#ifndef ANS_CONFIG_SAFETY_TYPES_H
#define ANS_CONFIG_SAFETY_TYPES_H

#include <stdbool.h>

/**
 * Safety limits and fallback behavior for daemon fan control.
 *
 * These settings bound manual, preset, and automatic control so bad sensor
 * reads or critical temperatures cannot leave the machine under-cooled.
 */
struct safety_config
{
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
