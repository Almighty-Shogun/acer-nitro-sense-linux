#ifndef ANS_FAN_SAFETY_H
#define ANS_FAN_SAFETY_H

#include "daemon/types.h"

int fan_filtered_sensor_temp(const char *fan_id, const char *source,
                             int *current_temp, int *pending_spike_temp,
                             int sensor_temp, bool trusted_baseline,
                             int critical_temperature_c);

#endif
