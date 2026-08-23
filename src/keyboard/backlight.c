#include "keyboard/backlight.h"

#include "util/string.h"
#include "keyboard/backlight_internal.h"

#include <string.h>

/**
 * Initialize a keyboard backlight status result.
 *
 * Callers start from a known unavailable state before probing sysfs or EC
 * backends, which keeps status output deterministic on unsupported systems.
 */
void keyboard_backlight_init_status(struct keyboard_backlight_status* status)
{
    memset(status, 0, sizeof(*status));
    string_copy(status->backend, sizeof(status->backend), "none");

    status->reg = -1;
    status->brightness = -1;
    status->max_brightness = -1;
    status->percent = -1;
}

/**
 * Convert a backend brightness value to a percentage.
 *
 * EC and sysfs backends can expose different brightness ranges. This normalizes
 * them to the public 0-100 status value.
 */
int keyboard_backlight_percent_from_range(const int value, const int min_value, const int max_value)
{
    const int range = max_value - min_value;

    if (range <= 0)
        return -1;

    return ((value - min_value) * 100 + range / 2) / range;
}

/**
 * Convert a percentage to the configured EC brightness value.
 *
 * The model profile defines the raw EC range, while users and integrations work
 * with percentages.
 */
int keyboard_backlight_value_from_percent(const struct keyboard_backlight_config* cfg, const int percent)
{
    const int range = cfg->max_value - cfg->min_value;

    return cfg->min_value + (percent * range + 50) / 100;
}

/**
 * Read keyboard backlight state from Linux sysfs.
 *
 * This portable path is tried before the model-specific EC fallback.
 */
bool keyboard_backlight_read(struct keyboard_backlight_status* status)
{
    return keyboard_backlight_read_sysfs(status);
}

/**
 * Read keyboard backlight state from any available backend.
 *
 * The sysfs backend wins when present. If the kernel does not expose a keyboard
 * LED and the model supports EC control, the EC backend is used instead.
 */
bool keyboard_backlight_read_any(struct ec_device* ec, const struct ans_config* cfg, struct keyboard_backlight_status* status)
{
    if (keyboard_backlight_read(status))
        return true;

    if (cfg && cfg->keyboard_backlight.available && ec)
        return keyboard_backlight_read_ec(ec, cfg, status);

    return false;
}

/**
 * Describe why keyboard backlight control is unavailable.
 *
 * The reason is used in diagnostics and capability output, so it stays short
 * and stable.
 */
const char* keyboard_backlight_reason(const struct keyboard_backlight_status* status)
{
    if (status->available)
        return "ok";

    if (strcmp(status->backend, "ec") == 0)
        return "ec-read-failed";

    if (status->name[0])
        return "sysfs-led-read-failed";

    return "no-sysfs-led";
}
