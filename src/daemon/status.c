#include "daemon/status.h"

#include "daemon/status_format.h"
#include "fan/control.h"
#include "keyboard/backlight.h"
#include "platform/control.h"
#include "platform/power_source.h"
#include "util/file.h"
#include "util/format.h"

#include <stdio.h>
#include <time.h>

void write_status(const struct ans_config *cfg, struct ec_device *ec,
                  const fan_state states[ANS_MAX_FANS], const bool auto_mode,
                  const char *preset, const bool coolboost_enabled,
                  const hardware_names *names,
                  const daemon_runtime_state *runtime)
{
    char buf[4096];
    text_buffer out;
    const time_t now = time(NULL);
    int fan_cpu_mode = -1;
    int fan_gpu_mode = -1;
    int platform_profile = -1;
    const enum power_source_state power_source = read_power_source();
    struct keyboard_backlight_status keyboard_backlight;
    const bool firmware_mode = firmware_auto_mode(auto_mode, preset);

    read_fan_mode(ec, cfg, &fan_cpu_mode, &fan_gpu_mode);
    read_platform_profile(ec, cfg, &platform_profile);
    keyboard_backlight_read_any(ec, cfg, &keyboard_backlight);

    text_buffer_init(&out, buf, sizeof(buf));
    text_buffer_append(
        &out,
        "{\n  \"model\": \"%s\",\n  \"backend\": \"%s\",\n"
        "  \"auto\": %s,\n  \"mode\": \"%s\",\n  \"preset\": \"%s\",\n"
        "  \"coolboost_available\": %s,\n  \"coolboost\": %s,\n"
        "  \"fan_mode_available\": %s,\n"
        "  \"fan_mode_cpu\": \"%s\",\n  \"fan_mode_gpu\": \"%s\",\n"
        "  \"platform_profile_available\": %s,\n"
        "  \"platform_profile\": \"%s\",\n"
        "  \"power_source\": \"%s\",\n"
        "  \"power_source_profile_policy_available\": %s,\n"
        "  \"power_source_profile_auto_apply\": %s,\n"
        "  \"power_source_ac_profile\": \"%s\",\n"
        "  \"power_source_battery_profile\": \"%s\",\n"
        "  \"keyboard_backlight_available\": %s,\n"
        "  \"keyboard_backlight_name\": \"%s\",\n"
        "  \"keyboard_backlight_brightness\": %d,\n"
        "  \"keyboard_backlight_max_brightness\": %d,\n"
        "  \"keyboard_backlight_percent\": %d,\n"
        "  \"keyboard_backlight_backend\": \"%s\",\n"
        "  \"keyboard_backlight_register\": %d,\n"
        "  \"keyboard_backlight_timeout_supported\": %s,\n"
        "  \"keyboard_backlight_timeout_enabled\": %s,\n"
        "  \"keyboard_backlight_timeout_seconds\": %d,\n"
        "  \"keyboard_backlight_timeout_backend\": \"%s\",\n"
        "  \"keyboard_backlight_timed_off\": %s,\n"
        "  \"keyboard_backlight_restore_percent\": %d,\n"
        "  \"keyboard_backlight_reason\": \"%s\",\n"
        "  \"timestamp\": %ld,\n  \"fans\": [\n",
        cfg->model, ec->name, auto_mode ? "true" : "false",
        control_mode(auto_mode, preset), preset,
        cfg->fan_modes.available ? "true" : "false",
        coolboost_enabled ? "true" : "false",
        cfg->fan_modes.available ? "true" : "false",
        cfg->fan_modes.available && fan_cpu_mode >= 0 ?
            fan_mode_value_name(&cfg->fan_modes, true, fan_cpu_mode) : "unavailable",
        cfg->fan_modes.available && fan_gpu_mode >= 0 ?
            fan_mode_value_name(&cfg->fan_modes, false, fan_gpu_mode) : "unavailable",
        cfg->platform_profiles.available ? "true" : "false",
        cfg->platform_profiles.available && platform_profile >= 0 ?
            platform_profile_value_name(cfg, platform_profile) : "unavailable",
        power_source_name(power_source),
        power_source_profile_policy_available(cfg) ? "true" : "false",
        runtime && runtime->power_source_auto_apply ? "true" : "false",
        cfg->power_source_profiles.ac_profile[0] ?
            cfg->power_source_profiles.ac_profile : "unavailable",
        cfg->power_source_profiles.battery_profile[0] ?
            cfg->power_source_profiles.battery_profile : "unavailable",
        keyboard_backlight.available ? "true" : "false",
        keyboard_backlight.name[0] ? keyboard_backlight.name : "unavailable",
        keyboard_backlight.brightness,
        keyboard_backlight.max_brightness,
        keyboard_backlight.percent,
        keyboard_backlight.backend[0] ? keyboard_backlight.backend : "none",
        keyboard_backlight.reg,
        cfg->keyboard_backlight.timeout_supported ? "true" : "false",
        runtime && runtime->keyboard_backlight_timeout_enabled ? "true" : "false",
        cfg->keyboard_backlight.timeout_supported ?
            cfg->keyboard_backlight.timeout_seconds : 0,
        cfg->keyboard_backlight.timeout_supported ? "input-activity" : "unsupported",
        runtime && runtime->keyboard_backlight_timed_off ? "true" : "false",
        runtime ? runtime->keyboard_backlight_restore_percent : -1,
        keyboard_backlight_reason(&keyboard_backlight),
        (long)now);

    for (int i = 0; i < cfg->fan_len; i++) {
        const char *control = status_fan_control_source(firmware_mode,
                                                        states[i].safety_active);
        char active_percent[16];

        if (firmware_mode && !states[i].safety_active)
            snprintf(active_percent, sizeof(active_percent), "null");
        else
            snprintf(active_percent, sizeof(active_percent), "%d", states[i].percent);

        text_buffer_append(
            &out,
            "    { \"id\": \"%s\", \"name\": \"%s\", \"component_name\": \"%s\", \"rpm\": %d, "
            "\"temp_c\": %d, \"sensor_temp_c\": %d, "
            "\"control_temp_c\": %d, \"control_sensor_temp_c\": %d, "
            "\"temp_available\": %s, \"control_temp_available\": %s, "
            "\"control\": \"%s\", \"active_percent\": %s, "
            "\"firmware_controlled\": %s, \"percent\": %d, "
            "\"requested_percent\": %d, \"effective_percent\": %d, "
            "\"write_value\": %d, "
            "\"critical_temp_samples\": %d, "
            "\"ec_read_failures\": %d, \"ec_write_failures\": %d, "
            "\"safety_active\": %s, "
            "\"safety_reason\": \"%s\" }%s\n",
            cfg->fans[i].id, cfg->fans[i].name,
            component_name_for_fan(names, &cfg->fans[i]), states[i].rpm,
            states[i].temp_c, states[i].sensor_temp_c,
            states[i].control_temp_c, states[i].control_sensor_temp_c,
            states[i].temp_available ? "true" : "false",
            states[i].control_temp_available ? "true" : "false",
            control, active_percent,
            firmware_mode && !states[i].safety_active ? "true" : "false",
            states[i].percent,
            states[i].requested_percent > 0 ?
                states[i].requested_percent : states[i].percent,
            states[i].percent,
            states[i].write_value,
            states[i].critical_temp_samples,
            states[i].ec_read_failures, states[i].ec_write_failures,
            states[i].safety_active ? "true" : "false", states[i].safety_reason,
            i == cfg->fan_len - 1 ? "" : ",");
    }

    text_buffer_append(&out, "  ]\n}\n");
    if (text_buffer_ok(&out))
        write_text_file_atomic(ANS_STATUS_PATH, buf);
}

void write_temperature_cache(const struct ans_config *cfg, const fan_state states[ANS_MAX_FANS])
{
    char buf[512];
    text_buffer out;

    text_buffer_init(&out, buf, sizeof(buf));
    text_buffer_append(&out, "{\n  \"fans\": [\n");

    for (int i = 0; i < cfg->fan_len; i++) {
        text_buffer_append(
            &out,
            "    { \"id\": \"%s\", \"temp_c\": %d, \"control_temp_c\": %d }%s\n",
            cfg->fans[i].id, states[i].temp_c, states[i].control_temp_c,
            i == cfg->fan_len - 1 ? "" : ",");
    }

    text_buffer_append(&out, "  ]\n}\n");
    if (text_buffer_ok(&out))
        write_text_file_atomic(ANS_TEMP_CACHE_PATH, buf);
}
