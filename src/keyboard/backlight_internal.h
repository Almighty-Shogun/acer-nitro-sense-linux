#ifndef ANS_KEYBOARD_BACKLIGHT_INTERNAL_H
#define ANS_KEYBOARD_BACKLIGHT_INTERNAL_H

#include "keyboard/backlight.h"

#include "config/types.h"

/**
 * Initialize a keyboard backlight status result.
 *
 * Callers start from this known unavailable state before trying sysfs or EC
 * backends.
 */
void keyboard_backlight_init_status(struct keyboard_backlight_status* status);

/**
 * Convert a backend brightness value to percent.
 *
 * The conversion honors the model's configured minimum and maximum EC values
 * instead of assuming a 0-100 backend range.
 */
int keyboard_backlight_percent_from_range(int value, int min_value, int max_value);

/**
 * Convert a percent value to a backend brightness value.
 *
 * This is the inverse of the range conversion used before EC brightness writes.
 */
int keyboard_backlight_value_from_percent(const struct keyboard_backlight_config* cfg, int percent);

/**
 * Read keyboard backlight state from Linux sysfs LEDs.
 *
 * This path is used when the kernel exposes a keyboard backlight LED device.
 */
bool keyboard_backlight_read_sysfs(struct keyboard_backlight_status* status);

/**
 * Read keyboard backlight state from the configured EC register.
 *
 * This path supports Nitro models that do not expose a kernel LED device for
 * keyboard lighting.
 */
bool keyboard_backlight_read_ec(struct ec_device* ec, const struct ans_config* cfg, struct keyboard_backlight_status* status);

#endif
