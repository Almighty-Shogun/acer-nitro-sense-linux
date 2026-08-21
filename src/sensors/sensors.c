#include "sensors/internal.h"
#include "sensors/sensors.h"
#include "util/file.h"
#include "util/string.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool sensor_group_matches(const char *group, const char *name)
{
    if (strcmp(group, "max") == 0)
        return sensor_group_matches("cpu", name) ||
               sensor_group_matches("gpu", name);

    if (strcmp(group, "cpu") == 0)
        return string_contains_case(name, "coretemp") ||
               string_contains_case(name, "k10temp") ||
               string_contains_case(name, "zenpower");

    if (strcmp(group, "gpu") == 0)
        return string_contains_case(name, "amdgpu") ||
               string_contains_case(name, "nvidia") ||
               string_contains_case(name, "nouveau") ||
               string_contains_case(name, "radeon");

    return false;
}

char *sensor_path_join(const char *left, const char *right)
{
    const size_t left_len = strlen(left);
    const size_t right_len = strlen(right);
    const bool needs_slash = left_len > 0 && left[left_len - 1] != '/';
    const size_t path_len = left_len + (needs_slash ? 1 : 0) + right_len + 1;
    char *path;

    path = malloc(path_len);
    if (!path)
        return NULL;

    snprintf(path, path_len, needs_slash ? "%s/%s" : "%s%s", left, right);
    return path;
}

static int read_temp_file_c(const char *path)
{
    char *text = read_text_file(path, 64);
    char *end;
    long milli;

    if (!text)
        return -1;

    errno = 0;
    milli = strtol(text, &end, 10);
    free(text);
    if (errno != 0 || end == text || milli <= 0)
        return -1;
    return (int)(milli / 1000);
}

int sensor_read_group_max_c(const char *group)
{
    int fake_temp = sensor_read_fake_temp_c(group);
    DIR *hwmon;
    struct dirent *entry;
    int best = -1;

    if (fake_temp >= 0)
        return fake_temp;

    hwmon = opendir("/sys/class/hwmon");
    if (!hwmon)
        return best;

    if (strcmp(group, "gpu") == 0 || strcmp(group, "max") == 0)
        best = sensor_read_nvidia_ml_c();

    while ((entry = readdir(hwmon))) {
        char *base;
        char *name_path;
        char *name;
        DIR *chip;
        struct dirent *temp_entry;

        if (entry->d_name[0] == '.')
            continue;

        base = sensor_path_join("/sys/class/hwmon", entry->d_name);
        if (!base)
            continue;
        name_path = sensor_path_join(base, "name");
        if (!name_path) {
            free(base);
            continue;
        }
        name = read_text_file(name_path, 128);
        free(name_path);
        if (!name) {
            free(base);
            continue;
        }

        if (!sensor_group_matches(group, name)) {
            free(name);
            free(base);
            continue;
        }
        free(name);

        chip = opendir(base);
        if (!chip) {
            free(base);
            continue;
        }

        while ((temp_entry = readdir(chip))) {
            char *temp_path;
            int temp;

            if (strncmp(temp_entry->d_name, "temp", 4) != 0 ||
                !string_contains_case(temp_entry->d_name, "_input"))
                continue;

            temp_path = sensor_path_join(base, temp_entry->d_name);
            if (!temp_path)
                continue;
            temp = read_temp_file_c(temp_path);
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
