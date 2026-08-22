#include "keyboard/backlight.h"

#include "keyboard/backlight_internal.h"
#include "util/string.h"

#include <string.h>

void keyboard_backlight_init_status(struct keyboard_backlight_status *status)
{
    memset(status, 0, sizeof(*status));
    string_copy(status->backend, sizeof(status->backend), "none");
    status->reg = -1;
    status->brightness = -1;
    status->max_brightness = -1;
    status->percent = -1;
}

int keyboard_backlight_percent_from_range(const int value, const int min_value,
                                          const int max_value)
{
    const int range = max_value - min_value;

    if (range <= 0)
        return -1;

    return ((value - min_value) * 100 + range / 2) / range;
}

int keyboard_backlight_value_from_percent(const struct keyboard_backlight_config *cfg,
                                          const int percent)
{
    const int range = cfg->max_value - cfg->min_value;

    return cfg->min_value + (percent * range + 50) / 100;
}

bool keyboard_backlight_read(struct keyboard_backlight_status *status)
{
    return keyboard_backlight_read_sysfs(status);
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
