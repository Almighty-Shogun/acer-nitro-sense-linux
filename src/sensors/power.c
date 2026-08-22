#include "sensors/internal.h"
#include "sensors/sensors.h"
#include "util/file.h"
#include "util/string.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int write_power_control(const char *path, const char *control)
{
    FILE *f = fopen(path, "we");
    int result;

    if (!f)
        return -1;

    result = fprintf(f, "%s\n", control) < 0 ? -1 : 0;
    if (fclose(f) < 0)
        return -1;
    return result;
}

static int read_power_control(const char *path, char *out, const size_t out_len)
{
    char *text = read_text_file(path, 64);

    if (!text)
        return -1;

    trim_ascii(text);
    if (!string_copy(out, out_len, text)) {
        free(text);
        return -1;
    }

    free(text);
    return 0;
}

int sensor_set_group_power_control(const char *group, const char *control)
{
    DIR *hwmon = opendir("/sys/class/hwmon");
    struct dirent *entry;
    int changed = 0;

    if (!hwmon)
        return -1;

    while ((entry = readdir(hwmon))) {
        char power_path[PATH_MAX];

        if (sensor_hwmon_entry_power_control_path(entry, group, power_path,
                                                  sizeof(power_path)) &&
            write_power_control(power_path, control) == 0)
            changed++;
    }

    closedir(hwmon);
    return changed;
}

int sensor_read_group_power_control(const char *group, char *out,
                                    const size_t out_len)
{
    DIR *hwmon = opendir("/sys/class/hwmon");
    struct dirent *entry;

    if (!hwmon)
        return -1;

    while ((entry = readdir(hwmon))) {
        char power_path[PATH_MAX];

        if (sensor_hwmon_entry_power_control_path(entry, group, power_path,
                                                  sizeof(power_path)) &&
            read_power_control(power_path, out, out_len) == 0) {
            closedir(hwmon);
            return 0;
        }
    }

    closedir(hwmon);
    return -1;
}
