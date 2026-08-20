#include "fan/control.h"
#include "fan/observation.h"
#include "fan/safety.h"
#include "platform/control.h"

#include <stdio.h>
#include <string.h>

static int curve_speed(const struct fan_config *fan, const int temp_c, const int current_percent)
{
    int selected = fan->curve_len > 0 ? fan->curve[0].speed : 50;

    for (int i = 0; i < fan->curve_len; i++) {
        if (temp_c >= fan->curve[i].up)
            selected = fan->curve[i].speed;
    }

    for (int i = 0; i < fan->curve_len; i++) {
        if (fan->curve[i].speed == current_percent && temp_c >= fan->curve[i].down)
            return current_percent > selected ? current_percent : selected;
    }

    return selected;
}

static bool any_safety_active(const struct ans_config *cfg,
                              const fan_state states[ANS_MAX_FANS])
{
    for (int i = 0; i < cfg->fan_len; i++) {
        if (states[i].safety_active)
            return true;
    }

    return false;
}

void update_fan_states(struct ec_device *ec, const struct ans_config *cfg,
                       fan_state states[ANS_MAX_FANS], const bool auto_mode,
                       const char *preset)
{
    const char *forced_reason = refresh_fan_observations(ec, cfg, states);
    const bool firmware_auto = firmware_auto_mode(auto_mode, preset);
    const bool firmware_auto_safety_was_active =
        firmware_auto && any_safety_active(cfg, states);
    bool firmware_auto_safety_is_active = false;
    bool firmware_auto_override_ready = false;

    for (int i = 0; i < cfg->fan_len; i++) {
        const struct fan_config *fan = &cfg->fans[i];
        const int temp_c = states[i].control_temp_c;
        int percent = states[i].requested_percent > 0 ?
            states[i].requested_percent : states[i].percent;

        if (auto_mode) {
            if (temp_c > 0) {
                const int curve_percent = curve_speed(fan, temp_c, percent);

                percent = auto_ramped_percent(cfg, &states[i], curve_percent,
                                              forced_reason);
            } else if (percent < fan->reset_speed) {
                percent = fan->reset_speed;
            }
        }

        const char *reason;
        const int effective_percent =
            fan_safety_adjust_percent(cfg, fan, &states[i], percent,
                                      forced_reason, &reason);

        if (firmware_auto) {
            if (reason[0] != '\0') {
                firmware_auto_safety_is_active = true;
                if (!firmware_auto_override_ready)
                    firmware_auto_override_ready =
                        apply_daemon_control_fan_mode(ec, cfg);

                if (!firmware_auto_override_ready && !daemon_quiet_logs)
                    fprintf(stderr,
                            "warning: failed to switch firmware-auto safety override to manual fan mode\n");

                set_fan_percent(ec, cfg, fan, &states[i], percent,
                                forced_reason);
                continue;
            }

            const int requested_percent = clamp_int(percent, 1, 100);

            states[i].requested_percent = requested_percent;
            states[i].percent = requested_percent;
            states[i].write_value = requested_percent;
            fan_update_safety_state(fan, &states[i], reason, requested_percent,
                                    requested_percent);
            continue;
        }

        if (effective_percent != states[i].percent || percent != states[i].requested_percent)
            set_fan_percent(ec, cfg, fan, &states[i], percent, forced_reason);
        else
            fan_update_safety_state(fan, &states[i], reason, percent,
                                    effective_percent);
    }

    if (firmware_auto && firmware_auto_safety_was_active &&
        !firmware_auto_safety_is_active &&
        !apply_firmware_auto_fan_mode(ec, cfg) && !daemon_quiet_logs)
        fprintf(stderr,
                "warning: failed to restore firmware-auto fan mode after safety override\n");
}

int set_one(struct ec_device *ec, const struct ans_config *cfg,
            fan_state states[ANS_MAX_FANS], const char *id, const int percent)
{
    int changed = 0;
    const char *forced_reason = global_safety_reason(cfg, states);

    for (int i = 0; i < cfg->fan_len; i++) {
        if (strcmp(id, "all") == 0 || strcmp(cfg->fans[i].id, id) == 0) {
            if (set_fan_percent(ec, cfg, &cfg->fans[i], &states[i], percent,
                                forced_reason) >= 0)
                changed++;
        }
    }

    return changed;
}

bool apply_preset(struct ec_device *ec, const struct ans_config *cfg,
                  fan_state states[ANS_MAX_FANS], const char *id)
{
    const struct preset_config *preset = config_find_preset(cfg, id);

    if (!preset)
        return false;

    set_one(ec, cfg, states, "cpu", preset->cpu);
    set_one(ec, cfg, states, "gpu", preset->gpu);

    return true;
}

void apply_current_control_state(struct ec_device *ec, const struct ans_config *cfg,
                                 fan_state states[ANS_MAX_FANS])
{
    const char *forced_reason = global_safety_reason(cfg, states);

    for (int i = 0; i < cfg->fan_len; i++) {
        const int percent = states[i].requested_percent > 0 ?
            states[i].requested_percent : states[i].percent;

        set_fan_percent(ec, cfg, &cfg->fans[i], &states[i], percent,
                        forced_reason);
    }
}
