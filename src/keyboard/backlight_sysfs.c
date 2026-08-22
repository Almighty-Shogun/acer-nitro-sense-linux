#include "keyboard/backlight_internal.h"

#include "util/file.h"
#include "util/string.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool candidate_name(const char *name)
{
    return string_contains_case(name, "kbd_backlight") ||
           (string_contains_case(name, "keyboard") &&
            string_contains_case(name, "backlight")) ||
           string_contains_case(name, "::kbd") ||
           string_contains_case(name, "platform::kbd");
}

static int read_int_file(const char *path)
{
    char *text = read_text_file(path, 64);
    char *end;
    long value;

    if (!text)
        return -1;

    value = strtol(text, &end, 10);
    free(text);
    if (end == text || value < 0 || value > 100000)
        return -1;

    return (int)value;
}

static bool init_sysfs_status(struct keyboard_backlight_status *status,
                              const char *name)
{
    const size_t name_len = strlen(name);

    if (name_len >= sizeof(status->name))
        return false;

    string_copy(status->name, sizeof(status->name), name);
    snprintf(status->path, sizeof(status->path), "/sys/class/leds/%s", name);
    return true;
}

bool keyboard_backlight_read_sysfs(struct keyboard_backlight_status *status)
{
    DIR *dir;
    struct dirent *entry;

    keyboard_backlight_init_status(status);
    string_copy(status->backend, sizeof(status->backend), "sysfs");

    dir = opendir("/sys/class/leds");
    if (!dir)
        return false;

    while ((entry = readdir(dir))) {
        char brightness_path[640];
        char max_brightness_path[640];

        if (entry->d_name[0] == '.' || !candidate_name(entry->d_name))
            continue;
        if (!init_sysfs_status(status, entry->d_name))
            continue;

        snprintf(brightness_path, sizeof(brightness_path), "%s/brightness",
                 status->path);
        snprintf(max_brightness_path, sizeof(max_brightness_path),
                 "%s/max_brightness", status->path);

        status->brightness = read_int_file(brightness_path);
        status->max_brightness = read_int_file(max_brightness_path);
        status->available = status->brightness >= 0 && status->max_brightness > 0;
        if (status->available)
            status->percent =
                keyboard_backlight_percent_from_range(status->brightness, 0,
                                                      status->max_brightness);
        closedir(dir);
        return status->available;
    }

    closedir(dir);
    return false;
}
