#include "keyboard/backlight_internal.h"

#include "ec/ec.h"
#include "util/string.h"

/**
 * Read keyboard backlight state from the configured EC register.
 *
 * Nitro models without a sysfs LED still expose brightness through model
 * profile EC values. Invalid or out-of-range reads are treated as unavailable.
 */
bool keyboard_backlight_read_ec(struct ec_device* ec, const struct ans_config* cfg, struct keyboard_backlight_status* status)
{
    keyboard_backlight_init_status(status);

    string_copy(status->backend, sizeof(status->backend), "ec");
    string_copy(status->name, sizeof(status->name), "acer-ec");

    status->reg = cfg->keyboard_backlight.reg;
    status->max_brightness = cfg->keyboard_backlight.max_value;

    if (!cfg->keyboard_backlight.available)
        return false;

    const int value = ec_read_byte(ec, cfg->keyboard_backlight.reg);

    if (value < cfg->keyboard_backlight.min_value || value > cfg->keyboard_backlight.max_value)
        return false;

    status->available = true;
    status->brightness = value;

    status->percent = keyboard_backlight_percent_from_range(
        value,
        cfg->keyboard_backlight.min_value,
        cfg->keyboard_backlight.max_value
    );

    return true;
}

/**
 * Set keyboard backlight brightness through the EC backend.
 *
 * The public percentage is converted into the model's configured EC range, then
 * read back so callers receive the actual resulting status.
 */
bool keyboard_backlight_set_percent(
    struct ec_device* ec,
    const struct ans_config* cfg,
    const int percent,
    struct keyboard_backlight_status* status
)
{
    const int range = cfg->keyboard_backlight.max_value - cfg->keyboard_backlight.min_value;

    const int value = keyboard_backlight_value_from_percent(&cfg->keyboard_backlight, percent);

    if (!cfg->keyboard_backlight.available || range <= 0)
        return false;

    if (ec_write_byte(ec, cfg->keyboard_backlight.reg, value) < 0)
        return false;

    return keyboard_backlight_read_ec(ec, cfg, status);
}
