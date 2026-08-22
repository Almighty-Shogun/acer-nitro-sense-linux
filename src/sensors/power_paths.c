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
        string_copy(path, sizeof(path), base);

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
                string_copy(out, out_len, candidate);
                free(candidate);
                return 0;
            }
            free(candidate);
        }

        parent_dir(path);
    }

    return -1;
}

bool sensor_hwmon_entry_power_control_path(const struct dirent *entry,
                                           const char *group, char *out,
                                           const size_t out_len)
{
    char *base;
    char *name_path;
    char *name;
    bool found = false;

    if (entry->d_name[0] == '.')
        return false;

    base = sensor_path_join("/sys/class/hwmon", entry->d_name);
    if (!base)
        return false;

    name_path = sensor_path_join(base, "name");
    if (!name_path) {
        free(base);
        return false;
    }

    name = read_text_file(name_path, 128);
    free(name_path);
    if (!name) {
        free(base);
        return false;
    }

    if (sensor_group_matches(group, name))
        found = find_power_control_path(base, out, out_len) == 0;

    free(name);
    free(base);
    return found;
}
