#include "util/file.h"
#include "util/string.h"
#include "sensors/sensors.h"
#include "sensors/internal.h"

#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

/**
 * Return whether a hwmon device name belongs to a logical sensor group.
 *
 * Model files refer to CPU and GPU groups instead of Linux driver names. This
 * mapper keeps the rest of the daemon independent from whether a system reports
 * temperatures through coretemp, nouveau, amdgpu, or another supported driver.
 */
bool sensor_group_matches(const char* group, const char* name)
{
    if (strcmp(group, "max") == 0)
        return sensor_group_matches("cpu", name) || sensor_group_matches("gpu", name);

    if (strcmp(group, "cpu") == 0)
        return string_contains_case(name, "coretemp")
            || string_contains_case(name, "k10temp")
            || string_contains_case(name, "zenpower");

    if (strcmp(group, "gpu") == 0)
        return string_contains_case(name, "amdgpu")
            || string_contains_case(name, "nvidia")
            || string_contains_case(name, "nouveau")
            || string_contains_case(name, "radeon");

    return false;
}

/**
 * Join two sensor sysfs path fragments.
 *
 * Sensor discovery builds many short sysfs paths while walking hwmon devices.
 * Returning an allocated string keeps those paths simple without requiring
 * fixed-size buffers in every caller.
 */
char* sensor_path_join(const char* left, const char* right)
{
    const size_t left_len = strlen(left);
    const size_t right_len = strlen(right);

    const bool needs_slash = left_len > 0 && left[left_len - 1] != '/';
    const size_t path_len = left_len + (needs_slash ? 1 : 0) + right_len + 1;

    char* path = malloc(path_len);

    if (!path)
        return NULL;

    snprintf(path, path_len, needs_slash ? "%s/%s" : "%s%s", left, right);

    return path;
}

/**
 * Read one hwmon temperature input as degrees Celsius.
 *
 * Linux reports hwmon temperatures in millidegrees. Invalid, missing, or
 * suspended sensor reads return `-1` so fan safety can distinguish unavailable
 * data from a real low temperature.
 */
static int read_temp_file_c(const char* path)
{
    char* end;
    char* text = read_text_file(path, 64);

    if (!text)
        return -1;

    errno = 0;

    const long milli = strtol(text, &end, 10);

    free(text);

    if (errno != 0 || end == text || milli <= 0)
        return -1;

    return (int)(milli / 1000);
}

/**
 * Read the hottest temperature in a sensor group.
 *
 * The daemon cools against the hottest value reported by the group. For NVIDIA
 * GPUs, NVML is sampled before hwmon so proprietary-driver systems can still
 * expose a temperature when sysfs is unavailable.
 */
int sensor_read_group_max_c(const char* group)
{
    struct dirent* entry;
    const int fake_temp = sensor_read_fake_temp_c(group);

    int best = -1;

    if (fake_temp >= 0)
        return fake_temp;

    DIR* hwmon = opendir("/sys/class/hwmon");

    if (!hwmon)
        return best;

    if (strcmp(group, "gpu") == 0 || strcmp(group, "max") == 0)
        best = sensor_read_nvidia_ml_c();

    while ((entry = readdir(hwmon)))
    {
        struct dirent* temp_entry;

        if (entry->d_name[0] == '.') continue;

        char* base = sensor_path_join("/sys/class/hwmon", entry->d_name);

        if (!base) continue;

        char* name_path = sensor_path_join(base, "name");

        if (!name_path)
        {
            free(base);

            continue;
        }

        char* name = read_text_file(name_path, 128);

        free(name_path);

        if (!name)
        {
            free(base);

            continue;
        }

        if (!sensor_group_matches(group, name))
        {
            free(name);
            free(base);

            continue;
        }

        free(name);

        DIR* chip = opendir(base);

        if (!chip)
        {
            free(base);

            continue;
        }

        while ((temp_entry = readdir(chip)))
        {
            if (strncmp(temp_entry->d_name, "temp", 4) != 0 || !string_contains_case(temp_entry->d_name, "_input")) continue;

            char* temp_path = sensor_path_join(base, temp_entry->d_name);

            if (!temp_path) continue;

            const int temp = read_temp_file_c(temp_path);

            free(temp_path);

            if (temp > best)
                best = temp;
        }

        closedir(chip);
        free(base);
    }

    closedir(hwmon);

    return best;
}
