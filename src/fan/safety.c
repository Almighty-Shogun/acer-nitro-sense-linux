#include "fan/safety.h"

#include "ec/ec.h"
#include "util/number.h"

#include <stdio.h>
#include <string.h>

void fan_update_safety_state(const struct fan_config *fan, fan_state *state,
                             const char *reason, const int requested_percent,
                             const int effective_percent)
{
    const bool active = reason && reason[0] != '\0';
    const bool changed = active != state->safety_active ||
                         (active && strcmp(state->safety_reason, reason) != 0);

    if (changed) {
        if (active) {
            if (!daemon_quiet_logs)
                fprintf(stderr,
                        "safety active fan=%s reason=%s requested=%d effective=%d temp=%d control_temp=%d rpm=%d ec_read_failures=%d ec_write_failures=%d\n",
                        fan->id, reason, requested_percent, effective_percent,
                        state->temp_c, state->control_temp_c, state->rpm,
                        state->ec_read_failures, state->ec_write_failures);
        } else {
            if (!daemon_quiet_logs)
                fprintf(stderr,
                        "safety cleared fan=%s previous=%s requested=%d effective=%d temp=%d control_temp=%d rpm=%d\n",
                        fan->id, state->safety_reason, requested_percent,
                        effective_percent, state->temp_c, state->control_temp_c,
                        state->rpm);
        }
    }

    state->safety_active = active;
    snprintf(state->safety_reason, sizeof(state->safety_reason), "%s",
             active ? reason : "");
}

int fan_safety_adjust_percent(const struct ans_config *cfg,
                              const struct fan_config *fan, fan_state *state,
                              const int requested_percent,
                              const char *forced_reason, const char **reason)
{
    int percent = clamp_int(requested_percent, 1, 100);

    *reason = "";

    if (forced_reason && forced_reason[0] != '\0') {
        *reason = forced_reason;
        if (strcmp(forced_reason, "critical-temperature") == 0 &&
            !cfg->safety.critical_full_speed)
            return clamp_int(percent + cfg->safety.critical_step_percent,
                             fan->write_min, fan->write_max);

        return clamp_int(cfg->safety.critical_speed_percent, fan->write_min,
                         fan->write_max);
    }

    if (state->ec_write_failures >= cfg->safety.max_ec_write_failures) {
        *reason = "ec-write-failure";
        return clamp_int(cfg->safety.critical_speed_percent, fan->write_min,
                         fan->write_max);
    }

    if (!state->control_temp_available) {
        const int fallback_percent = fan->missing_temperature_speed_percent > 0 ?
            fan->missing_temperature_speed_percent :
            cfg->safety.missing_temperature_speed_percent;

        if (percent < fallback_percent) {
            *reason = "temperature-unknown";
            percent = fallback_percent;
        }
        return percent;
    }

    if (state->critical_temp_samples >= cfg->safety.critical_consecutive_samples) {
        *reason = "critical-temperature";
        if (!cfg->safety.critical_full_speed)
            return clamp_int(percent + cfg->safety.critical_step_percent,
                             fan->write_min, fan->write_max);

        return clamp_int(cfg->safety.critical_speed_percent, fan->write_min,
                         fan->write_max);
    }

    if (state->control_temp_c >= cfg->safety.min_speed_temperature_c &&
        percent < cfg->safety.min_speed_percent) {
        *reason = "minimum-safe-speed";
        percent = cfg->safety.min_speed_percent;
    }

    return clamp_int(percent, fan->write_min, fan->write_max);
}

int fan_write_percent_raw(struct ec_device *ec, const struct fan_config *fan,
                          const int percent)
{
    const int value = clamp_int(percent, fan->write_min, fan->write_max);

    return ec_write_byte(ec, fan->write_register, value);
}

int set_fan_percent(struct ec_device *ec, const struct ans_config *cfg,
                    const struct fan_config *fan, fan_state *state,
                    const int requested_percent, const char *forced_reason)
{
    const char *reason;
    const int effective_percent =
        fan_safety_adjust_percent(cfg, fan, state, requested_percent,
                                  forced_reason, &reason);

    if (fan_write_percent_raw(ec, fan, effective_percent) < 0) {
        if (state->ec_write_failures < cfg->safety.max_ec_write_failures + 1)
            state->ec_write_failures++;
        fan_update_safety_state(fan, state, "ec-write-failure",
                                requested_percent, effective_percent);
        return -1;
    }

    if (!daemon_quiet_logs)
        fprintf(stderr,
                "ec_write fan=%s requested=%d effective=%d previous=%d safety=%s%s%s\n",
                fan->id, requested_percent, effective_percent, state->percent,
                reason[0] ? "active" : "ok", reason[0] ? " reason=" : "",
                reason[0] ? reason : "");

    state->ec_write_failures = 0;
    state->requested_percent = clamp_int(requested_percent, 1, 100);
    state->percent = effective_percent;
    state->write_value = effective_percent;
    fan_update_safety_state(fan, state, reason, requested_percent,
                            effective_percent);
    return effective_percent;
}

const char *global_safety_reason(const struct ans_config *cfg,
                                 const fan_state states[ANS_MAX_FANS])
{
    const char *reason = "";

    for (int i = 0; i < cfg->fan_len; i++) {
        if (states[i].ec_read_failures >= cfg->safety.max_ec_read_failures)
            return "ec-read-failure";
        if (states[i].ec_write_failures >= cfg->safety.max_ec_write_failures)
            return "ec-write-failure";
        if (states[i].critical_temp_samples >=
            cfg->safety.critical_consecutive_samples)
            reason = "critical-temperature";
    }

    return reason;
}

int auto_ramped_percent(const struct ans_config *cfg, const fan_state *state,
                        const int target_percent, const char *forced_reason)
{
    const int step = cfg->safety.auto_ramp_up_percent;
    const int current_percent = state->requested_percent > 0 ?
        state->requested_percent : state->percent;

    if (step <= 0 || target_percent <= current_percent)
        return target_percent;
    if (forced_reason && forced_reason[0] != '\0')
        return target_percent;
    if (cfg->safety.auto_ramp_bypass_temperature_c > 0 &&
        state->control_temp_available &&
        state->control_temp_c >= cfg->safety.auto_ramp_bypass_temperature_c)
        return target_percent;

    return clamp_int(current_percent + step, 1, target_percent);
}
