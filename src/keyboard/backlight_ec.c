#include "keyboard/backlight_internal.h"

#include "ec/ec.h"
#include "util/string.h"

bool keyboard_backlight_read_ec(struct ec_device *ec, const struct ans_config *cfg,
                                struct keyboard_backlight_status *status)
{
    int value;

    keyboard_backlight_init_status(status);
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
    status->percent =
        keyboard_backlight_percent_from_range(
            value, cfg->keyboard_backlight.min_value,
            cfg->keyboard_backlight.max_value);

    return true;
}

bool keyboard_backlight_set_percent(struct ec_device *ec,
                                    const struct ans_config *cfg,
                                    const int percent,
                                    struct keyboard_backlight_status *status)
{
    const int range = cfg->keyboard_backlight.max_value -
        cfg->keyboard_backlight.min_value;
    const int value =
        keyboard_backlight_value_from_percent(&cfg->keyboard_backlight, percent);

    if (!cfg->keyboard_backlight.available || range <= 0)
        return false;

    if (ec_write_byte(ec, cfg->keyboard_backlight.reg, value) < 0)
        return false;

    return keyboard_backlight_read_ec(ec, cfg, status);
}
