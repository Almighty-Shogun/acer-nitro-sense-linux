#ifndef ANS_SENSORS_INTERNAL_H
#define ANS_SENSORS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

struct dirent;

int sensor_read_fake_temp_c(const char *group);
int sensor_read_nvidia_ml_c(void);
bool sensor_group_matches(const char *group, const char *name);
char *sensor_path_join(const char *left, const char *right);
bool sensor_hwmon_entry_power_control_path(const struct dirent *entry,
                                           const char *group, char *out,
                                           size_t out_len);

#endif
