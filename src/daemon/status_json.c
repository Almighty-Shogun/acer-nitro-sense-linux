#include "daemon/status_json.h"

#include "daemon/status_json_internal.h"
#include "keyboard/backlight.h"
#include "platform/control.h"
#include "platform/power_source.h"
#include "util/format.h"
#include "util/json.h"

bool format_status_json(char *buf, const size_t buf_len,
                        const struct ans_config *cfg, struct ec_device *ec,
                        const fan_state states[ANS_MAX_FANS],
                        const bool auto_mode, const char *preset,
                        const bool coolboost_enabled,
                        const hardware_names *names,
                        const daemon_runtime_state *runtime,
                        const time_t now)
{
    text_buffer out;
    int fan_cpu_mode = -1;
    int fan_gpu_mode = -1;
    int platform_profile = -1;
    const enum power_source_state power_source = read_power_source();
    struct keyboard_backlight_status keyboard_backlight;
    const bool firmware_mode = firmware_auto_mode(auto_mode, preset);

    read_fan_mode(ec, cfg, &fan_cpu_mode, &fan_gpu_mode);
    read_platform_profile(ec, cfg, &platform_profile);
    keyboard_backlight_read_any(ec, cfg, &keyboard_backlight);

    text_buffer_init(&out, buf, buf_len);
    text_buffer_append(
        &out,
        "{\n  \"model\": ");
    json_append_string(&out, cfg->model);
    text_buffer_append(&out, ",\n  \"backend\": ");
    json_append_string(&out, ec->name);
    text_buffer_append(
        &out,
        ",\n  \"auto\": %s,\n  \"mode\": ",
        bool_text(auto_mode));
    json_append_string(&out, control_mode(auto_mode, preset));
    text_buffer_append(&out, ",\n  \"preset\": ");
    json_append_string(&out, preset);
    text_buffer_append(
        &out,
        ",\n"
        "  \"coolboost_available\": %s,\n  \"coolboost\": %s,\n"
        "  \"fan_mode_available\": %s,\n"
        "  \"fan_mode_cpu\": ",
        bool_text(cfg->fan_modes.available),
        bool_text(coolboost_enabled),
        bool_text(cfg->fan_modes.available));
    json_append_string(&out,
                       cfg->fan_modes.available && fan_cpu_mode >= 0 ?
                           fan_mode_value_name(&cfg->fan_modes, true, fan_cpu_mode) :
                           "unavailable");
    text_buffer_append(&out, ",\n  \"fan_mode_gpu\": ");
    json_append_string(&out,
                       cfg->fan_modes.available && fan_gpu_mode >= 0 ?
                           fan_mode_value_name(&cfg->fan_modes, false, fan_gpu_mode) :
                           "unavailable");
    text_buffer_append(
        &out,
        ",\n"
        "  \"platform_profile_available\": %s,\n"
        "  \"platform_profile\": ",
        bool_text(cfg->platform_profiles.available));
    json_append_string(&out,
                       cfg->platform_profiles.available && platform_profile >= 0 ?
                           platform_profile_value_name(cfg, platform_profile) :
                           "unavailable");
    text_buffer_append(&out, ",\n  \"power_source\": ");
    json_append_string(&out, power_source_name(power_source));
    text_buffer_append(
        &out,
        ",\n"
        "  \"power_source_profile_policy_available\": %s,\n"
        "  \"power_source_profile_auto_apply\": %s,\n"
        "  \"power_source_ac_profile\": ",
        bool_text(power_source_profile_policy_available(cfg)),
        bool_text(runtime && runtime->power_source_auto_apply));
    json_append_string(&out,
                       fallback_text(cfg->power_source_profiles.ac_profile,
                                     "unavailable"));
    text_buffer_append(&out, ",\n  \"power_source_battery_profile\": ");
    json_append_string(&out,
                       fallback_text(cfg->power_source_profiles.battery_profile,
                                     "unavailable"));
    text_buffer_append(
        &out,
        ",\n"
        "  \"keyboard_backlight_available\": %s,\n"
        "  \"keyboard_backlight_name\": ",
        bool_text(keyboard_backlight.available));
    json_append_string(&out,
                       fallback_text(keyboard_backlight.name, "unavailable"));
    text_buffer_append(
        &out,
        ",\n"
        "  \"keyboard_backlight_brightness\": %d,\n"
        "  \"keyboard_backlight_max_brightness\": %d,\n"
        "  \"keyboard_backlight_percent\": %d,\n"
        "  \"keyboard_backlight_backend\": ",
        keyboard_backlight.brightness,
        keyboard_backlight.max_brightness,
        keyboard_backlight.percent);
    json_append_string(&out, fallback_text(keyboard_backlight.backend, "none"));
    text_buffer_append(
        &out,
        ",\n"
        "  \"keyboard_backlight_register\": %d,\n"
        "  \"keyboard_backlight_timeout_supported\": %s,\n"
        "  \"keyboard_backlight_timeout_enabled\": %s,\n"
        "  \"keyboard_backlight_timeout_seconds\": %d,\n"
        "  \"keyboard_backlight_timeout_backend\": ",
        keyboard_backlight.reg,
        bool_text(cfg->keyboard_backlight.timeout_supported),
        bool_text(runtime && runtime->keyboard_backlight_timeout_enabled),
        cfg->keyboard_backlight.timeout_supported ?
            cfg->keyboard_backlight.timeout_seconds : 0);
    json_append_string(&out,
                       cfg->keyboard_backlight.timeout_supported ?
                           "input-activity" :
                           "unsupported");
    text_buffer_append(
        &out,
        ",\n"
        "  \"keyboard_backlight_timed_off\": %s,\n"
        "  \"keyboard_backlight_restore_percent\": %d,\n"
        "  \"keyboard_backlight_reason\": ",
        bool_text(runtime && runtime->keyboard_backlight_timed_off),
        runtime ? runtime->keyboard_backlight_restore_percent : -1);
    json_append_string(&out, keyboard_backlight_reason(&keyboard_backlight));
    text_buffer_append(
        &out,
        ",\n"
        "  \"timestamp\": %ld,\n  \"fans\": [\n",
        (long)now);

    for (int i = 0; i < cfg->fan_len; i++)
        append_fan_status_json(&out, cfg, states, names, firmware_mode, i);

    text_buffer_append(&out, "  ]\n}\n");
    return text_buffer_ok(&out);
}
