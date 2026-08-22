#include "daemon/diagnostics.h"

#include "ec/ec.h"
#include "util/string.h"
#include "sensors/sensors.h"
#include "hardware/hardware.h"
#include "platform/power_source.h"
#include "daemon/diagnostics_formats.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Format optional register.
 *
 * Daemon helpers own process lifetime and runtime coordination. They keep
 * privileged EC access behind one service instead of spreading it into
 * clients.
 */
static void format_optional_register(char* out, const size_t out_len, const int reg)
{
    if (reg >= 0)
    {
        snprintf(out, out_len, "0x%02x", reg);
    }
    else
    {
        string_copy(out, out_len, "none");
    }
}

/**
 * Read a fan temperature from its EC register.
 *
 * Daemon helpers own process lifetime and runtime coordination. They keep
 * privileged EC access behind one service instead of spreading it into
 * clients.
 */
static int fan_ec_temperature(struct ec_device* ec, const int reg)
{
    const int temp = reg >= 0 ? ec_read_byte(ec, reg) : -1;

    return temp > 0 && temp <= 130 ? temp : -1;
}

/**
 * Validate fan.
 *
 * Daemon helpers own process lifetime and runtime coordination. They keep
 * privileged EC access behind one service instead of spreading it into
 * clients.
 */
static void validate_fan(struct ec_device* ec, const struct ans_config* cfg, const struct fan_config* fan)
{
    char temperature_register[16], control_temperature_register[16];

    const int ec_temp = fan_ec_temperature(ec, fan->temperature_register);
    const int temp = ec_temp >= 0 ? ec_temp : sensor_read_group_max_c(fan->sensor_group);
    const int ec_control_temp = fan_ec_temperature(ec, fan->control_temperature_register);
    const int raw = cfg->read_words ? ec_read_word(ec, fan->read_register) : ec_read_byte(ec, fan->read_register);

    const int control_temp = ec_control_temp >= 0
                                 ? ec_control_temp
                                 : fan->control_sensor_group[0]
                                 ? sensor_read_group_max_c(fan->control_sensor_group)
                                 : temp;

    format_optional_register(temperature_register, sizeof(temperature_register), fan->temperature_register);
    format_optional_register(control_temperature_register, sizeof(control_temperature_register), fan->control_temperature_register);

    printf(
        DIAGNOSTIC_FORMAT_FAN,
        fan->id,
        fan->name,
        fan->sensor_group,
        fan->control_sensor_group[0] ? fan->control_sensor_group : fan->sensor_group,
        raw,
        temp,
        control_temp,
        fan->read_register,
        fan->write_register,
        temperature_register,
        control_temperature_register,
        fan->reset_speed,
        fan->missing_temperature_speed_percent > 0
            ? fan->missing_temperature_speed_percent
            : cfg->safety.missing_temperature_speed_percent,
        fan->curve_len,
        fan->keep_awake ? "true" : "false",
        fan->sensor_power_control[0] ? fan->sensor_power_control : "default"
    );
}

/**
 * Validate model.
 *
 * Daemon helpers own process lifetime and runtime coordination. They keep
 * privileged EC access behind one service instead of spreading it into
 * clients.
 */
void validate_model(struct ec_device* ec, const struct ans_config* cfg, const char* config_path, const bool force_model)
{
    const char* dmi = load_dmi_model();
    const bool allowed = force_model || dmi_allowed(cfg, dmi);

    printf("config=%s\n", config_path);
    printf("model=%s\n", cfg->model);
    printf("dmi=%s\n", dmi[0] ? dmi : "unknown");
    printf("dmi_allowed=%s\n", allowed ? "true" : "false");
    printf("backend=%s\n", ec->name);

    printf(
        "fans=%d presets=%d poll_interval_ms=%d critical_temperature_c=%d\n",
        cfg->fan_len,
        cfg->preset_len,
        cfg->poll_interval_ms,
        cfg->critical_temperature_c
    );

    printf(
        DIAGNOSTIC_FORMAT_SAFETY,
        cfg->safety.min_speed_percent, cfg->safety.min_speed_temperature_c,
        cfg->safety.critical_speed_percent,
        cfg->safety.critical_full_speed ? "true" : "false",
        cfg->safety.critical_step_percent,
        cfg->safety.critical_consecutive_samples,
        cfg->safety.critical_release_temperature_c,
        cfg->safety.auto_ramp_up_percent,
        cfg->safety.auto_ramp_bypass_temperature_c,
        cfg->safety.missing_temperature_speed_percent,
        cfg->safety.max_ec_read_failures, cfg->safety.max_ec_write_failures
    );

    printf(
        "coolboost available=%s backend=%s default_enabled=%s\n",
        cfg->fan_modes.available ? "true" : "false",
        cfg->fan_modes.available ? "fan-mode-turbo" : "unavailable",
        cfg->coolboost.default_enabled ? "true" : "false"
    );

    if (cfg->fan_modes.available)
    {
        printf(
            DIAGNOSTIC_FORMAT_FAN_MODES,
            cfg->fan_modes.cpu_reg, cfg->fan_modes.gpu_reg,
            cfg->fan_modes.cpu_auto_value, cfg->fan_modes.cpu_manual_value,
            cfg->fan_modes.cpu_turbo_value, cfg->fan_modes.gpu_auto_value,
            cfg->fan_modes.gpu_manual_value, cfg->fan_modes.gpu_turbo_value
        );
    }
    else
    {
        printf("fan_modes available=false\n");
    }

    if (cfg->platform_profiles.available)
    {
        printf(
            DIAGNOSTIC_FORMAT_PLATFORM_PROFILES,
            cfg->platform_profiles.reg,
            cfg->platform_profiles.default_profile[0] ? cfg->platform_profiles.default_profile : "none",
            cfg->platform_profiles.profile_len
        );

        for (int i = 0; i < cfg->platform_profiles.profile_len; i++)
            printf(
                "platform_profile=%s value=0x%02x\n",
                cfg->platform_profiles.profiles[i].id,
                cfg->platform_profiles.profiles[i].value
            );
    }
    else
    {
        printf("platform_profiles available=false\n");
    }

    printf(
        DIAGNOSTIC_FORMAT_POWER_SOURCE_PROFILES,
        power_source_profile_policy_available(cfg) ? "true" : "false",
        cfg->power_source_profiles.auto_apply ? "true" : "false",
        cfg->power_source_profiles.ac_profile[0] ? cfg->power_source_profiles.ac_profile : "none",
        cfg->power_source_profiles.battery_profile[0] ? cfg->power_source_profiles.battery_profile : "none",
        power_source_name(read_power_source())
    );

    for (int i = 0; i < cfg->fan_len; i++)
        validate_fan(ec, cfg, &cfg->fans[i]);
}
