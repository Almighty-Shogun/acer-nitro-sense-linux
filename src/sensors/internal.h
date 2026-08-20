#ifndef ANS_SENSORS_INTERNAL_H
#define ANS_SENSORS_INTERNAL_H

#include <stdbool.h>

int sensor_read_fake_temp_c(const char *group);
int sensor_read_nvidia_ml_c(void);
bool sensor_group_matches(const char *group, const char *name);
char *sensor_path_join(const char *left, const char *right);

#endif
