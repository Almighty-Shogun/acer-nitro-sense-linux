#include "daemon/state.h"

#include "util/file.h"
#include "util/json.h"
#include "fan/control.h"
#include "util/string.h"
#include "config/config.h"
#include "platform/control.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Compute saved percent for.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
static int json_saved_percent_for_fan(const char* json, const char* id, const int fallback)
{
    const char* fan_obj = json_object_with_id(json, id);

    if (!fan_obj)
        return fallback;

    const int percent = json_int_key(fan_obj, "percent", fallback);

    if (percent < 1 || percent > 100)
        return fallback;

    return percent;
}

/**
 * Restore runtime state from.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
static void restore_runtime_state_from_json(const struct ans_config* cfg, daemon_runtime_state* runtime, const char* json)
{
    if (!runtime) return;

    const bool keyboard_backlight_timeout_enabled = json_bool_key(
        json,
        "keyboard_backlight_timeout_enabled",
        cfg->keyboard_backlight.timeout_default_enabled
    );

    runtime->power_source_auto_apply = json_bool_key(json, "power_source_auto_apply", cfg->power_source_profiles.auto_apply);

    runtime->keyboard_backlight_timeout_enabled = cfg->keyboard_backlight.timeout_supported && keyboard_backlight_timeout_enabled;

    runtime->keyboard_backlight_timeout_seconds = cfg->keyboard_backlight.timeout_seconds;
}

/**
 * Apply saved percentages.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
static void apply_saved_fan_percentages(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    const char* json
)
{
    for (int i = 0; i < cfg->fan_len; i++)
    {
        const int percent = json_saved_percent_for_fan(json, cfg->fans[i].id, cfg->fans[i].reset_speed);

        set_fan_percent(ec, cfg, &cfg->fans[i], &states[i], percent, global_safety_reason(cfg, states));
    }
}

/**
 * Restore saved targets.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
static void restore_saved_fan_targets(const struct ans_config* cfg, fan_state states[ANS_MAX_FANS], const char* json)
{
    for (int i = 0; i < cfg->fan_len; i++)
    {
        const int percent = json_saved_percent_for_fan(json, cfg->fans[i].id, cfg->fans[i].reset_speed);

        states[i].percent = percent;
        states[i].requested_percent = percent;
    }
}

/**
 * Restore auto state.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
static void restore_auto_control_state(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    char* preset,
    const size_t preset_len,
    const char* json
)
{
    string_copy(preset, preset_len, "auto");
    apply_daemon_control_fan_mode(ec, cfg);
    apply_saved_fan_percentages(ec, cfg, states, json);
}

/**
 * Restore firmware-auto state.
 *
 * Firmware-auto leaves live fan speed to the Acer firmware, so saved targets
 * are restored for reporting only. The preset falls back to manual when the
 * firmware mode cannot be applied.
 */
static void restore_firmware_auto_control_state(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    char* preset,
    const size_t preset_len,
    const char* json
)
{
    string_copy(preset, preset_len, FIRMWARE_AUTO_PRESET);
    restore_saved_fan_targets(cfg, states, json);

    if (!apply_firmware_auto_fan_mode(ec, cfg))
        string_copy(preset, preset_len, "manual");
}

/**
 * Restore named preset state.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
static bool restore_named_preset_control_state(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    const char* preset
)
{
    if (strcmp(preset, "manual") == 0 || !config_find_preset(cfg, preset))
        return false;

    apply_daemon_control_fan_mode(ec, cfg);
    apply_preset(ec, cfg, states, preset);

    return true;
}

/**
 * Restore manual state.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
static void restore_manual_control_state(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    char* preset,
    const size_t preset_len,
    const char* json
)
{
    string_copy(preset, preset_len, "manual");

    apply_daemon_control_fan_mode(ec, cfg);
    apply_saved_fan_percentages(ec, cfg, states, json);
}

/**
 * Restore state from.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
bool restore_control_state_from_json(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* json
)
{
    *auto_mode = json_bool_key(json, "auto", true);
    (void)json_bool_key(json, "coolboost", cfg->coolboost.default_enabled);

    *coolboost_enabled = false;
    restore_runtime_state_from_json(cfg, runtime, json);

    string_copy(preset, preset_len, "auto");
    json_string_key(json, "preset", preset, preset_len);

    if (*auto_mode)
    {
        restore_auto_control_state(ec, cfg, states, preset, preset_len, json);
    }
    else if (strcmp(preset, FIRMWARE_AUTO_PRESET) == 0)
    {
        restore_firmware_auto_control_state(ec, cfg, states, preset, preset_len, json);
    }
    else if (!restore_named_preset_control_state(ec, cfg, states, preset))
    {
        restore_manual_control_state(ec, cfg, states, preset, preset_len, json);
    }

    if (cfg->fan_modes.available && !firmware_auto_mode(*auto_mode, preset))
        apply_coolboost(ec, cfg, states, false);

    if (!daemon_quiet_logs)
        fprintf(stderr, "restored control state: preset=%s auto=%d coolboost=0\n", preset, *auto_mode ? 1 : 0);

    return true;
}

/**
 * Restore state.
 *
 * Persisted state lets the daemon restart without surprising the user with a
 * different fan mode. Restore paths intentionally validate before applying
 * saved values.
 */
bool restore_control_state(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime
)
{
    char* json = read_text_file(ANS_STATE_PATH, 64 * 1024);

    if (!json)
        return false;

    const bool restored = restore_control_state_from_json(
        ec,
        cfg,
        states,
        auto_mode,
        preset,
        preset_len,
        coolboost_enabled,
        runtime,
        json
    );

    free(json);

    return restored;
}
