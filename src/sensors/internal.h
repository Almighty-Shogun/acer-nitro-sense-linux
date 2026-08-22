#ifndef ANS_SENSORS_INTERNAL_H
#define ANS_SENSORS_INTERNAL_H

#include <stddef.h>
#include <stdbool.h>

struct dirent;

/**
 * Read a fake sensor temperature for tests.
 *
 * Fake sensor values let unit tests exercise fan behavior without depending on
 * host hwmon devices.
 */
int sensor_read_fake_temp_c(const char* group);

/**
 * Read NVIDIA GPU temperature through NVML.
 *
 * NVML is used as a fallback when hwmon cannot report a live NVIDIA
 * temperature.
 */
int sensor_read_nvidia_ml_c(void);

/**
 * Return whether a hwmon name belongs to a sensor group.
 *
 * Group matching maps Linux device names such as coretemp or nouveau to the
 * model's configured CPU and GPU sensor groups.
 */
bool sensor_group_matches(const char* group, const char* name);

/**
 * Join two sensor sysfs path fragments.
 *
 * The caller owns the returned allocation.
 */
char* sensor_path_join(const char* left, const char* right);

/**
 * Find the runtime power-control path for a hwmon entry.
 *
 * hwmon entries are often nested below the PCI device, so this helper resolves
 * the corresponding power/control file for the sensor group.
 */
bool sensor_hwmon_entry_power_control_path(const struct dirent* entry, const char* group, char* out, size_t out_len);

#endif
