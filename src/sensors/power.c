#include "sensors/internal.h"
#include "util/file.h"
#include "util/string.h"

#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void parent_dir(char *path)
{
    char *slash = strrchr(path, '/');

    if (!slash || slash == path) {
        snprintf(path, PATH_MAX, "/");
        return;
    }
    *slash = '\0';
}

static bool file_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

static bool is_pci_device_dir_name(const char *name)
{
    const size_t len = strlen(name);

    if (len != 12 || name[4] != ':' || name[7] != ':' || name[10] != '.')
        return false;

    for (size_t i = 0; i < len; i++) {
        if (i == 4 || i == 7 || i == 10)
            continue;
        if (!isxdigit((unsigned char)name[i]))
            return false;
    }

    return true;
}

static int find_power_control_path(const char *base, char *out,
                                   const size_t out_len)
{
    char path[PATH_MAX];

    if (!realpath(base, path))
        snprintf(path, sizeof(path), "%s", base);

    while (strcmp(path, "/") != 0) {
        const char *name = strrchr(path, '/');

        name = name ? name + 1 : path;
        if (is_pci_device_dir_name(name)) {
            char *candidate = sensor_path_join(path, "power/control");

            if (!candidate)
                return -1;
            if (file_exists(candidate)) {
                if (strlen(candidate) >= out_len) {
                    free(candidate);
                    return -1;
                }
                snprintf(out, out_len, "%s", candidate);
                free(candidate);
                return 0;
            }
            free(candidate);
        }

        parent_dir(path);
    }

    return -1;
}

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
    if (strlen(text) >= out_len) {
        free(text);
        return -1;
    }

    snprintf(out, out_len, "%s", text);
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
        char *base;
        char *name_path;
        char power_path[PATH_MAX];
        char *name;

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

        if (find_power_control_path(base, power_path, sizeof(power_path)) == 0 &&
            write_power_control(power_path, control) == 0)
            changed++;
        free(base);
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
        char *base;
        char *name_path;
        char power_path[PATH_MAX];
        char *name;

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

        if (find_power_control_path(base, power_path, sizeof(power_path)) == 0 &&
            read_power_control(power_path, out, out_len) == 0) {
            free(base);
            closedir(hwmon);
            return 0;
        }
        free(base);
    }

    closedir(hwmon);
    return -1;
}
