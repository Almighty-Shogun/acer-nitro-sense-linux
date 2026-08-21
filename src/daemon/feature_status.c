#include "daemon/feature_status.h"

#include "config/config.h"
#include "control/protocol.h"
#include "ec/ec.h"
#include "keyboard/backlight.h"
#include "platform/control.h"
#include "platform/power_source.h"
#include "sensors/sensors.h"
#include "util/format.h"
#include "util/string.h"

#include <stdio.h>
#include <string.h>

void reply_coolboost_status(const int client, const struct ans_config *cfg,
                            const bool coolboost_enabled)
{
    control_reply(client, "coolboost=%s\n",
            cfg->fan_modes.available ?
                on_off_text(coolboost_enabled) :
                "unavailable");
}

void reply_fan_mode_status(const int client, struct ec_device *ec,
                           const struct ans_config *cfg)
{
    int cpu_value = -1;
    int gpu_value = -1;

    if (!read_fan_mode(ec, cfg, &cpu_value, &gpu_value)) {
        control_reply(client, "fan_mode=unavailable\n");
        return;
    }

    const char *cpu_mode = fan_mode_value_name(&cfg->fan_modes, true, cpu_value);
    const char *gpu_mode = fan_mode_value_name(&cfg->fan_modes, false, gpu_value);
    const char *mode = strcmp(cpu_mode, gpu_mode) == 0 ? cpu_mode : "mixed";

    control_reply(client,
            "fan_mode=%s cpu=%s cpu_value=0x%02x gpu=%s gpu_value=0x%02x\n",
            mode, cpu_mode, cpu_value, gpu_mode, gpu_value);
}

void reply_profile_status(const int client, struct ec_device *ec,
                          const struct ans_config *cfg)
{
    int value = -1;

    if (!read_platform_profile(ec, cfg, &value)) {
        control_reply(client, "profile=unavailable\n");
        return;
    }

    control_reply(client, "profile=%s value=0x%02x\n",
            platform_profile_value_name(cfg, value), value);
}

void reply_power_source_status(const int client, struct ec_device *ec,
                               const struct ans_config *cfg,
                               const daemon_runtime_state *runtime)
{
    const enum power_source_state source = read_power_source();
    int value = -1;

    read_platform_profile(ec, cfg, &value);
    control_reply(client,
            "power_source=%s policy=%s auto_apply=%s ac_profile=%s battery_profile=%s current_profile=%s target_profile=%s\n",
            power_source_name(source),
            availability_text(power_source_profile_policy_available(cfg)),
            on_off_text(runtime && runtime->power_source_auto_apply),
            fallback_text(cfg->power_source_profiles.ac_profile, "unavailable"),
            fallback_text(cfg->power_source_profiles.battery_profile, "unavailable"),
            cfg->platform_profiles.available && value >= 0 ?
                platform_profile_value_name(cfg, value) : "unavailable",
            power_source_profile_for(cfg, source) ?
                power_source_profile_for(cfg, source) : "unavailable");
}

static int read_gpu_ec_temperature_c(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     char *source, const size_t source_len)
{
    const struct fan_config *fan = config_find_fan(cfg, "gpu");
    const int reg = fan ? fan->temperature_register : -1;
    const int temp = reg >= 0 ? ec_read_byte(ec, reg) : -1;

    if (reg < 0 || temp <= 0 || temp > 130)
        return -1;

    snprintf(source, source_len, "ec:0x%02x", reg);
    return temp;
}

void reply_gpu_temp_status(const int client, struct ec_device *ec,
                           const struct ans_config *cfg)
{
    char control[16];
    char source[32] = "hwmon";
    int temp_c = read_gpu_ec_temperature_c(ec, cfg, source, sizeof(source));
    const bool has_power_control =
        sensor_read_group_power_control("gpu", control, sizeof(control)) == 0;

    if (temp_c < 0)
        temp_c = sensor_read_group_max_c("gpu");

    if (temp_c >= 0) {
        control_reply(client,
                "gpu_temp=available policy=%s live=%s temp=%dC readable=1 source=%s reason=ok\n",
                has_power_control ? control : "unavailable",
                has_power_control ?
                    (strcmp(control, "on") == 0 ? "on" : "auto") :
                    "unavailable",
                temp_c, source);
        return;
    }

    if (!has_power_control) {
        control_reply(client,
                "gpu_temp=available policy=unknown live=unknown temp=-- readable=0 reason=no-power-control\n");
        return;
    }

    control_reply(client,
            "gpu_temp=available policy=%s live=%s temp=-- readable=0 reason=sensor-unreadable\n",
            control, strcmp(control, "on") == 0 ? "on" : "auto");
}

void reply_keyboard_backlight_status(const int client, struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     const daemon_runtime_state *runtime)
{
    struct keyboard_backlight_status keyboard_backlight;
    char reg_text[16];
    const bool timeout_supported = cfg->keyboard_backlight.timeout_supported;
    const bool timeout_enabled = runtime &&
        runtime->keyboard_backlight_timeout_enabled;

    keyboard_backlight_read_any(ec, cfg, &keyboard_backlight);
    if (keyboard_backlight.reg >= 0)
        snprintf(reg_text, sizeof(reg_text), "0x%02x", keyboard_backlight.reg);
    else
        string_copy(reg_text, sizeof(reg_text), "unavailable");

    control_reply(client,
            "keyboard_backlight=%s timeout=%s timeout_seconds=%d timeout_backend=%s timed_off=%s restore_percent=%d name=%s brightness=%d max_brightness=%d percent=%d backend=%s register=%s reason=%s\n",
            availability_text(keyboard_backlight.available),
            timeout_supported ? on_off_text(timeout_enabled) : "unsupported",
            timeout_supported ? cfg->keyboard_backlight.timeout_seconds : 0,
            timeout_supported ? "input-activity" : "unsupported",
            runtime && runtime->keyboard_backlight_timed_off ? "yes" : "no",
            runtime ? runtime->keyboard_backlight_restore_percent : -1,
            fallback_text(keyboard_backlight.name, "unavailable"),
            keyboard_backlight.brightness,
            keyboard_backlight.max_brightness,
            keyboard_backlight.percent,
            fallback_text(keyboard_backlight.backend, "none"),
            reg_text,
            keyboard_backlight_reason(&keyboard_backlight));
}
