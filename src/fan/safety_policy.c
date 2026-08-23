#include "fan/internal.h"

#include "util/number.h"

#include <string.h>

/**
 * Return whether value.
 *
 * Safety code is intentionally conservative because it can override requested
 * fan speeds. The helper keeps thermal protection separate from normal preset
 * and curve selection.
 */
static bool text_has_value(const char* text)
{
    return text && text[0] != '\0';
}

/**
 * Compute critical safety percent.
 *
 * Safety code is intentionally conservative because it can override requested
 * fan speeds. The helper keeps thermal protection separate from normal preset
 * and curve selection.
 */
static int critical_safety_percent(
    const struct ans_config* cfg,
    const struct fan_config* fan,
    const int percent,
    const char* reason
)
{
    if (strcmp(reason, "critical-temperature") == 0 && !cfg->safety.critical_full_speed)
        return clamp_int(percent + cfg->safety.critical_step_percent, fan->write_min, fan->write_max);

    return clamp_int(cfg->safety.critical_speed_percent, fan->write_min, fan->write_max);
}

/**
 * Compute missing temperature percent.
 *
 * Safety code is intentionally conservative because it can override requested
 * fan speeds. The helper keeps thermal protection separate from normal preset
 * and curve selection.
 */
static int missing_temperature_percent(const struct ans_config* cfg, const struct fan_config* fan)
{
    return fan->missing_temperature_speed_percent > 0
               ? fan->missing_temperature_speed_percent
               : cfg->safety.missing_temperature_speed_percent;
}

/**
 * Compute safety adjust percent.
 *
 * Safety code is intentionally conservative because it can override requested
 * fan speeds. The helper keeps thermal protection separate from normal preset
 * and curve selection.
 */
int fan_safety_adjust_percent(
    const struct ans_config* cfg,
    const struct fan_config* fan,
    const fan_state* state,
    const int requested_percent,
    const char* forced_reason,
    const char** reason
)
{
    int percent = clamp_int(requested_percent, 1, 100);

    *reason = "";

    if (text_has_value(forced_reason))
    {
        *reason = forced_reason;

        return critical_safety_percent(cfg, fan, percent, forced_reason);
    }

    if (state->ec_write_failures >= cfg->safety.max_ec_write_failures)
    {
        *reason = "ec-write-failure";

        return critical_safety_percent(cfg, fan, percent, *reason);
    }

    if (!state->control_temp_available)
    {
        const int fallback_percent = missing_temperature_percent(cfg, fan);

        if (percent < fallback_percent)
        {
            *reason = "temperature-unknown";
            percent = fallback_percent;
        }

        return percent;
    }

    if (state->critical_temp_samples >= cfg->safety.critical_consecutive_samples)
    {
        *reason = "critical-temperature";

        return critical_safety_percent(cfg, fan, percent, *reason);
    }

    if (state->control_temp_c >= cfg->safety.min_speed_temperature_c && percent < cfg->safety.min_speed_percent)
    {
        *reason = "minimum-safe-speed";

        percent = cfg->safety.min_speed_percent;
    }

    return clamp_int(percent, fan->write_min, fan->write_max);
}
