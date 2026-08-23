#include "util/file.h"
#include "util/string.h"
#include "sensors/sensors.h"
#include "sensors/internal.h"

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

/**
 * Write a runtime power policy to one hwmon power/control file.
 *
 * The NVIDIA GPU sensor can suspend when idle. Writing `on` or `auto` lets the
 * daemon choose between live temperature reads and normal runtime power saving.
 */
static int write_power_control(const char* path, const char* control)
{
    FILE* f = fopen(path, "we");

    if (!f)
        return -1;

    const int result = fprintf(f, "%s\n", control) < 0 ? -1 : 0;

    if (fclose(f) < 0)
        return -1;

    return result;
}

/**
 * Read and trim one hwmon runtime power policy.
 *
 * The returned text mirrors Linux sysfs values such as `auto` and `on`.
 */
static int read_power_control(const char* path, char* out, const size_t out_len)
{
    char* text = read_text_file(path, 64);

    if (!text)
        return -1;

    trim_ascii(text);

    if (!string_copy(out, out_len, text))
    {
        free(text);

        return -1;
    }

    free(text);

    return 0;
}

/**
 * Write power control for one matching hwmon entry.
 *
 * Non-matching entries are ignored. A matching entry counts as changed only
 * when the sysfs write succeeds.
 */
static bool write_entry_power_control(const struct dirent* entry, const char* group, const char* control)
{
    char power_path[PATH_MAX];

    if (!sensor_hwmon_entry_power_control_path(entry, group, power_path, sizeof(power_path)))
        return false;

    return write_power_control(power_path, control) == 0;
}

/**
 * Read power control for one matching hwmon entry.
 *
 * The first readable matching entry becomes the group power policy reported to
 * callers.
 */
static bool read_entry_power_control(const struct dirent* entry, const char* group, char* out, const size_t out_len)
{
    char power_path[PATH_MAX];

    if (!sensor_hwmon_entry_power_control_path(entry, group, power_path, sizeof(power_path)))
        return false;

    return read_power_control(power_path, out, out_len) == 0;
}

/**
 * Set runtime power control for a sensor group.
 *
 * Every hwmon device matching the group receives the requested policy. The
 * return value is the number of successful writes, or `-1` when hwmon cannot be
 * opened.
 */
int sensor_set_group_power_control(const char* group, const char* control)
{
    DIR* hwmon = opendir("/sys/class/hwmon");

    int changed = 0;
    struct dirent* entry;

    if (!hwmon)
        return -1;

    while ((entry = readdir(hwmon)))
    {
        if (write_entry_power_control(entry, group, control))
            changed++;
    }

    closedir(hwmon);

    return changed;
}

/**
 * Read runtime power control for a sensor group.
 *
 * This returns the first readable policy for the matching group because all
 * devices in that group are expected to share the same requested policy.
 */
int sensor_read_group_power_control(const char* group, char* out, const size_t out_len)
{
    struct dirent* entry;

    DIR* hwmon = opendir("/sys/class/hwmon");

    if (!hwmon)
        return -1;

    while ((entry = readdir(hwmon)))
    {
        if (read_entry_power_control(entry, group, out, out_len))
        {
            closedir(hwmon);

            return 0;
        }
    }

    closedir(hwmon);

    return -1;
}
