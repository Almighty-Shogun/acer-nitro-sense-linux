#ifndef ANS_SENSORS_SENSORS_H
#define ANS_SENSORS_SENSORS_H

#include <stddef.h>

/**
 * Read the hottest temperature in a sensor group.
 *
 * Fan control uses the group maximum so multi-sensor components are cooled for
 * their hottest reported point.
 */
int sensor_read_group_max_c(const char* group);

/**
 * Set runtime power control for a sensor group.
 *
 * GPU temperature visibility can require keeping a runtime-suspended device
 * awake through its Linux power-control setting.
 */
int sensor_set_group_power_control(const char* group, const char* control);

/**
 * Read runtime power control for a sensor group.
 *
 * Diagnostics use this to report whether a sensor group is in automatic or
 * always-on runtime-power mode.
 */
int sensor_read_group_power_control(const char* group, char* out, size_t out_len);

#endif
