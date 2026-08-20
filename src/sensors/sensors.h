#ifndef ANS_SENSORS_SENSORS_H
#define ANS_SENSORS_SENSORS_H

#include <stddef.h>

int sensor_read_group_max_c(const char *group);
int sensor_set_group_power_control(const char *group, const char *control);
int sensor_read_group_power_control(const char *group, char *out, size_t out_len);

#endif
