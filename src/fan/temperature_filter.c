#include "fan/safety.h"

#include <stdio.h>
#include <stdlib.h>

enum {
    TEMP_SPIKE_FILTER_DELTA_C = 15,
    TEMP_SPIKE_ACCEPT_DELTA_C = 3,
};

int fan_filtered_sensor_temp(const char *fan_id, const char *source,
                             int *current_temp, int *pending_spike_temp,
                             const int sensor_temp, const bool trusted_baseline,
                             const int critical_temperature_c)
{
    if (sensor_temp < 0) {
        *pending_spike_temp = 0;

        return sensor_temp;
    }

    if (!trusted_baseline && sensor_temp >= critical_temperature_c) {
        if (*pending_spike_temp > 0 &&
            abs(sensor_temp - *pending_spike_temp) <=
                TEMP_SPIKE_ACCEPT_DELTA_C) {
            *pending_spike_temp = 0;

            return sensor_temp;
        }

        *pending_spike_temp = sensor_temp;
        if (!daemon_quiet_logs)
            fprintf(stderr,
                    "temperature_spike fan=%s source=%s raw=%d filtered=unknown\n",
                    fan_id, source, sensor_temp);

        return -1;
    }

    if (*current_temp > 0 && sensor_temp > *current_temp &&
        sensor_temp - *current_temp >= TEMP_SPIKE_FILTER_DELTA_C) {
        if (*pending_spike_temp > 0 &&
            abs(sensor_temp - *pending_spike_temp) <=
                TEMP_SPIKE_ACCEPT_DELTA_C) {
            *pending_spike_temp = 0;

            return sensor_temp;
        }

        *pending_spike_temp = sensor_temp;
        if (!daemon_quiet_logs)
            fprintf(stderr,
                    "temperature_spike fan=%s source=%s raw=%d filtered=%d\n",
                    fan_id, source, sensor_temp, *current_temp);

        return *current_temp;
    }

    *pending_spike_temp = 0;

    return sensor_temp;
}
