#include "daemon/state.h"

#include "fan/control.h"
#include "platform/control.h"
#include "util/json.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int json_saved_percent_for_fan(const char *json, const char *id,
                                      const int fallback)
{
    const char *fan_obj = json_object_with_id(json, id);

    if (!fan_obj)
        return fallback;

    const int percent = json_int_key(fan_obj, "percent", fallback);
    if (percent < 1 || percent > 100)
        return fallback;

    return percent;
}

void write_control_state(const struct ans_config *cfg,
                         const fan_state states[ANS_MAX_FANS],
                         const bool auto_mode, const char *preset,
                         const bool coolboost_enabled,
                         const daemon_runtime_state *runtime)
{
    char buf[1024];
    int off = 0;
    const time_t now = time(NULL);

    (void)coolboost_enabled;

    if (!daemon_persist_control_state)
        return;

    if (mkdir_p(ANS_STATE_DIR) < 0) {
        fprintf(stderr, "warning: failed to create state directory: %s\n",
                strerror(errno));
        return;
    }

    off += snprintf(buf + off, sizeof(buf) - (size_t)off,
                    "{\n  \"schema\": 1,\n  \"mode\": \"%s\",\n"
                    "  \"auto\": %s,\n  \"preset\": \"%s\",\n"
                    "  \"coolboost\": false,\n"
                    "  \"power_source_auto_apply\": %s,\n"
                    "  \"keyboard_backlight_timeout_enabled\": %s,\n"
                    "  \"timestamp\": %ld,\n  \"fans\": [\n",
                    control_mode(auto_mode, preset), auto_mode ? "true" : "false",
                    preset, runtime && runtime->power_source_auto_apply ? "true" : "false",
                    runtime && runtime->keyboard_backlight_timeout_enabled ?
                        "true" : "false",
                    (long)now);

    for (int i = 0; i < cfg->fan_len; i++) {
        const int percent = states[i].requested_percent > 0 ?
            states[i].requested_percent : states[i].percent;

        off += snprintf(buf + off, sizeof(buf) - (size_t)off,
                        "    { \"id\": \"%s\", \"percent\": %d }%s\n",
                        cfg->fans[i].id, percent,
                        i == cfg->fan_len - 1 ? "" : ",");
    }

    snprintf(buf + off, sizeof(buf) - (size_t)off, "  ]\n}\n");

    if (write_text_file_atomic(ANS_STATE_PATH, buf) < 0)
        fprintf(stderr, "warning: failed to save control state: %s\n",
                strerror(errno));
}

bool restore_control_state_from_json(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     fan_state states[ANS_MAX_FANS],
                                     bool *auto_mode, char *preset,
                                     const size_t preset_len,
                                     bool *coolboost_enabled,
                                     daemon_runtime_state *runtime,
                                     const char *json)
{
    *auto_mode = json_bool_key(json, "auto", true);
    (void)json_bool_key(json, "coolboost", cfg->coolboost.default_enabled);
    *coolboost_enabled = false;
    if (runtime)
        runtime->power_source_auto_apply =
            json_bool_key(json, "power_source_auto_apply",
                          cfg->power_source_profiles.auto_apply);
    if (runtime) {
        runtime->keyboard_backlight_timeout_enabled =
            cfg->keyboard_backlight.timeout_supported &&
            json_bool_key(json, "keyboard_backlight_timeout_enabled",
                          cfg->keyboard_backlight.timeout_default_enabled);
        runtime->keyboard_backlight_timeout_seconds =
            cfg->keyboard_backlight.timeout_seconds;
    }

    snprintf(preset, preset_len, "auto");
    json_string_key(json, "preset", preset, preset_len);

    if (*auto_mode) {
        snprintf(preset, preset_len, "auto");
        apply_daemon_control_fan_mode(ec, cfg);

        for (int i = 0; i < cfg->fan_len; i++) {
            const int percent = json_saved_percent_for_fan(
                json, cfg->fans[i].id, cfg->fans[i].reset_speed);

            set_fan_percent(ec, cfg, &cfg->fans[i], &states[i], percent,
                            global_safety_reason(cfg, states));
        }
    } else if (strcmp(preset, FIRMWARE_AUTO_PRESET) == 0) {
        snprintf(preset, preset_len, "%s", FIRMWARE_AUTO_PRESET);

        for (int i = 0; i < cfg->fan_len; i++) {
            const int percent = json_saved_percent_for_fan(
                json, cfg->fans[i].id, cfg->fans[i].reset_speed);

            states[i].percent = percent;
            states[i].requested_percent = percent;
        }

        if (!apply_firmware_auto_fan_mode(ec, cfg))
            snprintf(preset, preset_len, "manual");
    } else if (strcmp(preset, "manual") != 0 &&
               config_find_preset(cfg, preset)) {
        apply_daemon_control_fan_mode(ec, cfg);
        apply_preset(ec, cfg, states, preset);
    } else {
        snprintf(preset, preset_len, "manual");
        apply_daemon_control_fan_mode(ec, cfg);

        for (int i = 0; i < cfg->fan_len; i++) {
            const int percent = json_saved_percent_for_fan(
                json, cfg->fans[i].id, cfg->fans[i].reset_speed);

            set_fan_percent(ec, cfg, &cfg->fans[i], &states[i], percent,
                            global_safety_reason(cfg, states));
        }
    }

    if (cfg->fan_modes.available && !firmware_auto_mode(*auto_mode, preset))
        apply_coolboost(ec, cfg, states, false);

    if (!daemon_quiet_logs)
        fprintf(stderr, "restored control state: preset=%s auto=%d coolboost=0\n",
                preset, *auto_mode ? 1 : 0);

    return true;
}

bool restore_control_state(struct ec_device *ec, const struct ans_config *cfg,
                           fan_state states[ANS_MAX_FANS], bool *auto_mode,
                           char *preset, const size_t preset_len,
                           bool *coolboost_enabled,
                           daemon_runtime_state *runtime)
{
    char *json = read_text_file(ANS_STATE_PATH, 64 * 1024);
    bool restored;

    if (!json)
        return false;

    restored = restore_control_state_from_json(ec, cfg, states, auto_mode, preset,
                                               preset_len, coolboost_enabled,
                                               runtime, json);
    free(json);
    return restored;
}
