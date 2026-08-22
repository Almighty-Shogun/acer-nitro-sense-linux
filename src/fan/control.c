#include "fan/control.h"

#include "util/number.h"
#include "fan/internal.h"
#include "config/config.h"
#include "fan/observation.h"
#include "platform/control.h"

#include <stdio.h>
#include <string.h>

/**
 * Compute the fan curve target speed.
 *
 * The curve uses upward and downward thresholds to avoid fan oscillation
 * around a boundary. Existing fan speed can hold when hysteresis says cooling
 * should continue.
 */
static int curve_speed(const struct fan_config* fan, const int temp_c, const int current_percent)
{
    int selected = fan->curve_len > 0 ? fan->curve[0].speed : 50;

    for (int i = 0; i < fan->curve_len; i++)
    {
        if (temp_c >= fan->curve[i].up)
            selected = fan->curve[i].speed;
    }

    for (int i = 0; i < fan->curve_len; i++)
    {
        if (fan->curve[i].speed == current_percent && temp_c >= fan->curve[i].down)
            return current_percent > selected ? current_percent : selected;
    }

    return selected;
}

/**
 * Return whether any fan is under safety control.
 *
 * Fan control is the core cooling state machine. Helpers in this module
 * translate model curves, user requests, and safety policy into EC write
 * values.
 */
static bool any_safety_active(const struct ans_config* cfg, const fan_state states[ANS_MAX_FANS])
{
    for (int i = 0; i < cfg->fan_len; i++)
    {
        if (states[i].safety_active)
            return true;
    }

    return false;
}

/**
 * Return whether firmware-auto should be restored after a safety override.
 *
 * Firmware-auto is temporarily switched to daemon fan control while safety
 * policy needs to force an EC write. Once every fan leaves safety control, the
 * daemon should hand fan mode back to firmware.
 */
static bool should_restore_firmware_auto_mode(const bool firmware_auto, const bool safety_was_active, const bool safety_is_active)
{
    return firmware_auto && safety_was_active && !safety_is_active;
}

/**
 * Restore firmware-auto fan mode when a safety override ends.
 *
 * This keeps the warning path outside the main reconciliation loop and makes
 * the restore condition explicit instead of embedding every state check in one
 * long branch.
 */
static void restore_firmware_auto_mode_if_needed(
    struct ec_device* ec,
    const struct ans_config* cfg,
    const bool firmware_auto,
    const bool safety_was_active,
    const bool safety_is_active
)
{
    if (!should_restore_firmware_auto_mode(firmware_auto, safety_was_active, safety_is_active))
        return;

    if (!apply_firmware_auto_fan_mode(ec, cfg) && !daemon_quiet_logs)
        fprintf(stderr, "warning: failed to restore firmware-auto fan mode after safety override\n");
}

/**
 * Reconcile fan targets with sensors and safety policy.
 *
 * This is the main cooling reconciliation step: it samples temperatures,
 * applies curve or preset intent, and lets safety policy override unsafe
 * requests before EC writes.
 */
void update_fan_states(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset
)
{
    const bool firmware_auto = firmware_auto_mode(auto_mode, preset);
    const char* forced_reason = refresh_fan_observations(ec, cfg, states);

    const bool firmware_auto_safety_was_active = firmware_auto && any_safety_active(cfg, states);

    bool firmware_auto_override_ready = false;
    bool firmware_auto_safety_is_active = false;

    for (int i = 0; i < cfg->fan_len; i++)
    {
        const int temp_c = states[i].control_temp_c;
        const struct fan_config* fan = &cfg->fans[i];

        int percent = states[i].requested_percent > 0 ? states[i].requested_percent : states[i].percent;

        if (auto_mode)
        {
            if (temp_c > 0)
            {
                const int curve_percent = curve_speed(fan, temp_c, percent);

                percent = auto_ramped_percent(cfg, &states[i], curve_percent, forced_reason);
            }
            else if (percent < fan->reset_speed)
            {
                percent = fan->reset_speed;
            }
        }

        const char* reason;
        const int effective_percent = fan_safety_adjust_percent(cfg, fan, &states[i], percent, forced_reason, &reason);

        if (firmware_auto)
        {
            if (reason[0] != '\0')
            {
                firmware_auto_safety_is_active = true;

                if (!firmware_auto_override_ready)
                    firmware_auto_override_ready = apply_daemon_control_fan_mode(ec, cfg);

                if (!firmware_auto_override_ready && !daemon_quiet_logs)
                    fprintf(stderr, "warning: failed to switch firmware-auto safety override to manual fan mode\n");

                set_fan_percent(ec, cfg, fan, &states[i], percent, forced_reason);

                continue;
            }

            const int requested_percent = clamp_int(percent, 1, 100);

            states[i].percent = requested_percent;
            states[i].write_value = requested_percent;
            states[i].requested_percent = requested_percent;

            fan_update_safety_state(fan, &states[i], reason, requested_percent, requested_percent);

            continue;
        }

        if (effective_percent != states[i].percent || percent != states[i].requested_percent)
        {
            set_fan_percent(ec, cfg, fan, &states[i], percent, forced_reason);
        }
        else
        {
            fan_update_safety_state(fan, &states[i], reason, percent, effective_percent);
        }
    }

    restore_firmware_auto_mode_if_needed(
        ec,
        cfg,
        firmware_auto,
        firmware_auto_safety_was_active,
        firmware_auto_safety_is_active
    );
}

/**
 * Set one fan target or every fan target.
 *
 * Manual fan commands share this path so cpu, gpu, and all targets receive
 * the same safety reason and write handling.
 */
int set_one(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS], const char* id, const int percent)
{
    int changed = 0;
    const char* forced_reason = global_safety_reason(cfg, states);

    for (int i = 0; i < cfg->fan_len; i++)
    {
        if (strcmp(id, "all") == 0 || strcmp(cfg->fans[i].id, id) == 0)
        {
            if (set_fan_percent(ec, cfg, &cfg->fans[i], &states[i], percent, forced_reason) >= 0)
                changed++;
        }
    }

    return changed;
}

/**
 * Apply a named fan preset from the model configuration.
 *
 * Named presets are model configuration, not hard-coded daemon behavior.
 * Looking them up here keeps command handling independent of profile
 * contents.
 */
bool apply_preset(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS], const char* id)
{
    const struct preset_config* preset = config_find_preset(cfg, id);

    if (!preset)
        return false;

    set_one(ec, cfg, states, "cpu", preset->cpu);
    set_one(ec, cfg, states, "gpu", preset->gpu);

    return true;
}

/**
 * Reapply the saved fan control state.
 *
 * Resume paths call this after restoring state so the EC reflects the daemon
 * state that users see in status output.
 */
void apply_current_control_state(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    const char* forced_reason = global_safety_reason(cfg, states);

    for (int i = 0; i < cfg->fan_len; i++)
    {
        const int percent = states[i].requested_percent > 0 ? states[i].requested_percent : states[i].percent;

        set_fan_percent(ec, cfg, &cfg->fans[i], &states[i], percent, forced_reason);
    }
}
