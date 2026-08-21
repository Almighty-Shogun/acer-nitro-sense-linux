#include "keyboard/backlight.h"

#include "config/types.h"
#include "ec/ec.h"
#include "util/file.h"
#include "util/string.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void init_status(struct keyboard_backlight_status *status)
{
    memset(status, 0, sizeof(*status));
    string_copy(status->backend, sizeof(status->backend), "none");
    status->reg = -1;
    status->brightness = -1;
    status->max_brightness = -1;
    status->percent = -1;
}

static bool candidate_name(const char *name)
{
    return string_contains_case(name, "kbd_backlight") ||
           (string_contains_case(name, "keyboard") &&
            string_contains_case(name, "backlight")) ||
           string_contains_case(name, "::kbd") ||
           string_contains_case(name, "platform::kbd");
}

static int percent_from_range(const int value, const int min_value,
                              const int max_value)
{
    const int range = max_value - min_value;

    if (range <= 0)
        return -1;

    return ((value - min_value) * 100 + range / 2) / range;
}

static int value_from_percent(const struct keyboard_backlight_config *cfg,
                              const int percent)
{
    const int range = cfg->max_value - cfg->min_value;

    return cfg->min_value + (percent * range + 50) / 100;
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

bool keyboard_backlight_read(struct keyboard_backlight_status *status)
{
    DIR *dir;
    struct dirent *entry;

    init_status(status);
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
                percent_from_range(status->brightness, 0,
                                   status->max_brightness);
        closedir(dir);
        return status->available;
    }

    closedir(dir);
    return false;
}

static bool keyboard_backlight_read_ec(struct ec_device *ec,
                                       const struct ans_config *cfg,
                                       struct keyboard_backlight_status *status)
{
    int value;

    init_status(status);
    string_copy(status->backend, sizeof(status->backend), "ec");
    string_copy(status->name, sizeof(status->name), "acer-ec");
    status->reg = cfg->keyboard_backlight.reg;
    status->max_brightness = cfg->keyboard_backlight.max_value;

    if (!cfg->keyboard_backlight.available)
        return false;

    value = ec_read_byte(ec, cfg->keyboard_backlight.reg);
    if (value < cfg->keyboard_backlight.min_value ||
        value > cfg->keyboard_backlight.max_value)
        return false;

    status->available = true;
    status->brightness = value;
    status->percent = percent_from_range(value,
                                         cfg->keyboard_backlight.min_value,
                                         cfg->keyboard_backlight.max_value);

    return true;
}

bool keyboard_backlight_read_any(struct ec_device *ec, const struct ans_config *cfg,
                                 struct keyboard_backlight_status *status)
{
    if (keyboard_backlight_read(status))
        return true;

    if (cfg && cfg->keyboard_backlight.available && ec)
        return keyboard_backlight_read_ec(ec, cfg, status);

    return false;
}

bool keyboard_backlight_set_percent(struct ec_device *ec,
                                    const struct ans_config *cfg,
                                    const int percent,
                                    struct keyboard_backlight_status *status)
{
    const int range = cfg->keyboard_backlight.max_value -
        cfg->keyboard_backlight.min_value;
    const int value = value_from_percent(&cfg->keyboard_backlight, percent);

    if (!cfg->keyboard_backlight.available || range <= 0)
        return false;

    if (ec_write_byte(ec, cfg->keyboard_backlight.reg, value) < 0)
        return false;

    return keyboard_backlight_read_ec(ec, cfg, status);
}

const char *keyboard_backlight_reason(const struct keyboard_backlight_status *status)
{
    if (status->available)
        return "ok";
    if (strcmp(status->backend, "ec") == 0)
        return "ec-read-failed";
    if (status->name[0])
        return "sysfs-led-read-failed";
    return "no-sysfs-led";
}
