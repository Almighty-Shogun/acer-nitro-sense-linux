#ifndef ANS_FAN_SAFETY_H
#define ANS_FAN_SAFETY_H

#include "daemon/types.h"

/**
 * Filter an incoming sensor temperature sample.
 *
 * The filter suppresses isolated impossible spikes while still accepting a
 * sustained jump when the next sample confirms it.
 */
int fan_filtered_sensor_temp(
    const char* fan_id,
    const char* source,
    const int* current_temp,
    int* pending_spike_temp,
    int sensor_temp,
    bool trusted_baseline,
    int critical_temperature_c
);

#endif
