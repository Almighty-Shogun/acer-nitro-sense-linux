#include "fan/safety.h"

#include "ec/ec.h"
#include "fan/control.h"
#include "util/number.h"
#include "fan/internal.h"

#include <stdio.h>

/**
 * Return whether a safety reason string is present.
 *
 * Empty strings mean normal fan curve behavior can still be ramped gradually.
 * A concrete reason means the daemon should apply the target immediately.
 */
static bool text_has_value(const char* text)
{
    return text && text[0] != '\0';
}

/**
 * Return whether auto ramping should be bypassed for the current temperature.
 *
 * Above the configured bypass temperature, the fan target is applied directly
 * instead of stepping toward it over multiple polling intervals.
 */
static bool auto_ramp_bypass_temperature_reached(const struct ans_config* cfg, const fan_state* state)
{
    return cfg->safety.auto_ramp_bypass_temperature_c > 0
           && state->control_temp_available
           && state->control_temp_c >= cfg->safety.auto_ramp_bypass_temperature_c;
}

/**
 * Write a clamped fan percentage to the EC.
 *
 * This function intentionally skips state updates; callers use it as the raw
 * hardware write primitive after safety policy has selected an effective
 * percentage.
 */
int fan_write_percent_raw(struct ec_device* ec, const struct fan_config* fan, const int percent)
{
    const int value = clamp_int(percent, fan->write_min, fan->write_max);

    return ec_write_byte(ec, fan->write_register, value);
}

/**
 * Apply a fan target after safety adjustment.
 *
 * The requested percentage is preserved for status output, while the effective
 * percentage records what was actually written after safety policy clamping or
 * escalation.
 */
int set_fan_percent(
    struct ec_device* ec,
    const struct ans_config* cfg,
    const struct fan_config* fan,
    fan_state* state,
    const int requested_percent,
    const char* forced_reason
)
{
    const char* reason;
    const int effective_percent = fan_safety_adjust_percent(cfg, fan, state, requested_percent, forced_reason, &reason);

    if (fan_write_percent_raw(ec, fan, effective_percent) < 0)
    {
        if (state->ec_write_failures < cfg->safety.max_ec_write_failures + 1)
            state->ec_write_failures++;

        fan_update_safety_state(fan, state, "ec-write-failure", requested_percent, effective_percent);

        return -1;
    }

    if (!daemon_quiet_logs)
        fprintf(
            stderr,
            "ec_write fan=%s requested=%d effective=%d previous=%d safety=%s%s%s\n",
            fan->id,
            requested_percent,
            effective_percent,
            state->percent,
            reason[0] ? "active" : "ok",
            reason[0] ? " reason=" : "",
            reason[0] ? reason : ""
        );

    state->ec_write_failures = 0;
    state->requested_percent = clamp_int(requested_percent, 1, 100);

    state->percent = effective_percent;
    state->write_value = effective_percent;

    fan_update_safety_state(fan, state, reason, requested_percent, effective_percent);

    return effective_percent;
}

/**
 * Find the global fan safety reason.
 *
 * EC read/write failures take priority because they affect every fan decision.
 * Critical temperature is returned only when no EC failure already forced a
 * safer global state.
 */
const char* global_safety_reason(const struct ans_config* cfg, const fan_state states[ANS_MAX_FANS])
{
    const char* reason = "";

    for (int i = 0; i < cfg->fan_len; i++)
    {
        if (states[i].ec_read_failures >= cfg->safety.max_ec_read_failures)
            return "ec-read-failure";

        if (states[i].ec_write_failures >= cfg->safety.max_ec_write_failures)
            return "ec-write-failure";

        if (states[i].critical_temp_samples >= cfg->safety.critical_consecutive_samples)
            reason = "critical-temperature";
    }

    return reason;
}

/**
 * Compute the next automatic fan percentage.
 *
 * Normal auto mode ramps upward gradually to avoid sudden fan jumps. Safety
 * reasons and high control temperatures bypass that ramp so cooling can react
 * immediately.
 */
int auto_ramped_percent(const struct ans_config* cfg, const fan_state* state, const int target_percent, const char* forced_reason)
{
    const int step = cfg->safety.auto_ramp_up_percent;
    const int current_percent = state->requested_percent > 0 ? state->requested_percent : state->percent;

    if (step <= 0 || target_percent <= current_percent)
        return target_percent;

    if (text_has_value(forced_reason))
        return target_percent;

    if (auto_ramp_bypass_temperature_reached(cfg, state))
        return target_percent;

    return clamp_int(current_percent + step, 1, target_percent);
}
