#ifndef ANS_KEYBOARD_BACKLIGHT_H
#define ANS_KEYBOARD_BACKLIGHT_H

#include <stdbool.h>

struct ans_config;
struct ec_device;

/**
 * Current keyboard backlight state and backend metadata.
 *
 * Status output uses this structure to report whether control is available,
 * which backend supplied the value, and the normalized percentage.
 */
struct keyboard_backlight_status
{
    bool available;
    char name[128];
    char path[512];
    char backend[32];
    int reg;
    int brightness;
    int max_brightness;
    int percent;
};

/**
 * Read keyboard backlight state through generic Linux backlight paths.
 *
 * This is the portable path for systems that expose keyboard lighting through
 * sysfs LED devices.
 */
bool keyboard_backlight_read(struct keyboard_backlight_status* status);

/**
 * Read keyboard backlight state through EC or generic Linux paths.
 *
 * Model profiles may define EC-backed brightness even when Linux does not
 * expose a keyboard LED device.
 */
bool keyboard_backlight_read_any(struct ec_device* ec, const struct ans_config* cfg, struct keyboard_backlight_status* status);

/**
 * Set keyboard backlight brightness to a supported percentage.
 *
 * The percentage is converted to the model's configured EC brightness range
 * before writing.
 */
bool keyboard_backlight_set_percent(
    struct ec_device* ec,
    const struct ans_config* cfg,
    int percent,
    struct keyboard_backlight_status* status
);

/**
 * Describe why keyboard backlight control is unavailable.
 *
 * The returned text is stable status output for diagnostics and integrations.
 */
const char* keyboard_backlight_reason(const struct keyboard_backlight_status* status);

#endif
