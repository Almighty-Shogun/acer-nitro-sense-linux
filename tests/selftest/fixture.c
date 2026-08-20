#include "selftest/fixture.h"

#include <stdio.h>
#include <string.h>

void init_self_test_config(struct ans_config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    snprintf(cfg->model, sizeof(cfg->model), "Acer Nitro Self Test");
    snprintf(cfg->default_preset, sizeof(cfg->default_preset), "auto");
    snprintf(cfg->ec_path, sizeof(cfg->ec_path), "fake");
    cfg->poll_interval_ms = 100;
    cfg->critical_temperature_c = 90;
    cfg->read_words = true;
    cfg->safety.min_speed_percent = 35;
    cfg->safety.min_speed_temperature_c = 60;
    cfg->safety.critical_speed_percent = 100;
    cfg->safety.critical_full_speed = true;
    cfg->safety.critical_step_percent = 20;
    cfg->safety.critical_consecutive_samples = 1;
    cfg->safety.critical_release_temperature_c = 85;
    cfg->safety.auto_ramp_up_percent = 12;
    cfg->safety.auto_ramp_bypass_temperature_c = 0;
    cfg->safety.missing_temperature_speed_percent = 60;
    cfg->safety.max_ec_read_failures = 3;
    cfg->safety.max_ec_write_failures = 3;
    cfg->coolboost.default_enabled = false;
    cfg->fan_modes.available = true;
    cfg->fan_modes.cpu_reg = 0x22;
    cfg->fan_modes.cpu_auto_value = 0x04;
    cfg->fan_modes.cpu_manual_value = 0x0c;
    cfg->fan_modes.cpu_turbo_value = 0x08;
    cfg->fan_modes.gpu_reg = 0x21;
    cfg->fan_modes.gpu_auto_value = 0x10;
    cfg->fan_modes.gpu_manual_value = 0x30;
    cfg->fan_modes.gpu_turbo_value = 0x20;
    cfg->platform_profiles.available = true;
    cfg->platform_profiles.reg = 0x2c;
    snprintf(cfg->platform_profiles.default_profile,
             sizeof(cfg->platform_profiles.default_profile), "balanced");
    cfg->platform_profiles.profile_len = 3;
    snprintf(cfg->platform_profiles.profiles[0].id,
             sizeof(cfg->platform_profiles.profiles[0].id), "quiet");
    cfg->platform_profiles.profiles[0].value = 0x00;
    snprintf(cfg->platform_profiles.profiles[1].id,
             sizeof(cfg->platform_profiles.profiles[1].id), "balanced");
    cfg->platform_profiles.profiles[1].value = 0x01;
    snprintf(cfg->platform_profiles.profiles[2].id,
             sizeof(cfg->platform_profiles.profiles[2].id), "performance");
    cfg->platform_profiles.profiles[2].value = 0x04;
    cfg->power_source_profiles.auto_apply = false;
    snprintf(cfg->power_source_profiles.ac_profile,
             sizeof(cfg->power_source_profiles.ac_profile), "balanced");
    snprintf(cfg->power_source_profiles.battery_profile,
             sizeof(cfg->power_source_profiles.battery_profile), "quiet");
    cfg->keyboard_backlight.available = true;
    cfg->keyboard_backlight.reg = 0x31;
    cfg->keyboard_backlight.min_value = 0;
    cfg->keyboard_backlight.max_value = 4;
    cfg->keyboard_backlight.timeout_supported = true;
    cfg->keyboard_backlight.timeout_default_enabled = false;
    cfg->keyboard_backlight.timeout_seconds = 30;
    cfg->fan_len = 2;

    snprintf(cfg->fans[0].id, sizeof(cfg->fans[0].id), "cpu");
    snprintf(cfg->fans[0].name, sizeof(cfg->fans[0].name), "CPU fan");
    snprintf(cfg->fans[0].sensor_group, sizeof(cfg->fans[0].sensor_group), "cpu");
    snprintf(cfg->fans[0].control_sensor_group,
             sizeof(cfg->fans[0].control_sensor_group), "max");
    cfg->fans[0].read_register = 0x13;
    cfg->fans[0].write_register = 0x37;
    cfg->fans[0].temperature_register = -1;
    cfg->fans[0].control_temperature_register = -1;
    cfg->fans[0].write_min = 0;
    cfg->fans[0].write_max = 100;
    cfg->fans[0].reset_speed = 50;
    cfg->fans[0].missing_temperature_speed_percent = 62;
    cfg->fans[0].curve_len = 2;
    cfg->fans[0].curve[0] = (struct threshold){.up = 0, .down = 0, .speed = 30};
    cfg->fans[0].curve[1] = (struct threshold){.up = 60, .down = 55, .speed = 55};

    snprintf(cfg->fans[1].id, sizeof(cfg->fans[1].id), "gpu");
    snprintf(cfg->fans[1].name, sizeof(cfg->fans[1].name), "GPU fan");
    snprintf(cfg->fans[1].sensor_group, sizeof(cfg->fans[1].sensor_group), "gpu");
    snprintf(cfg->fans[1].control_sensor_group,
             sizeof(cfg->fans[1].control_sensor_group), "max");
    cfg->fans[1].read_register = 0x15;
    cfg->fans[1].write_register = 0x3a;
    cfg->fans[1].temperature_register = -1;
    cfg->fans[1].control_temperature_register = -1;
    cfg->fans[1].write_min = 0;
    cfg->fans[1].write_max = 100;
    cfg->fans[1].reset_speed = 50;
    cfg->fans[1].curve_len = 2;
    cfg->fans[1].curve[0] = (struct threshold){.up = 0, .down = 0, .speed = 25};
    cfg->fans[1].curve[1] = (struct threshold){.up = 60, .down = 55, .speed = 55};

    cfg->preset_len = 2;
    snprintf(cfg->presets[0].id, sizeof(cfg->presets[0].id), "balanced");
    cfg->presets[0].cpu = 50;
    cfg->presets[0].gpu = 45;
    snprintf(cfg->presets[1].id, sizeof(cfg->presets[1].id), "max");
    cfg->presets[1].cpu = 100;
    cfg->presets[1].gpu = 100;
}

void reset_self_test_states(const struct ans_config *cfg, fan_state states[ANS_MAX_FANS])
{
    memset(states, 0, sizeof(fan_state) * ANS_MAX_FANS);
    for (int i = 0; i < cfg->fan_len; i++) {
        states[i].rpm = -1;
        states[i].temp_c = 45;
        states[i].sensor_temp_c = 45;
        states[i].control_temp_c = 45;
        states[i].control_sensor_temp_c = 45;
        states[i].temp_available = true;
        states[i].control_temp_available = true;
        states[i].temp_seeded = true;
        states[i].control_temp_seeded = true;
        states[i].percent = cfg->fans[i].reset_speed;
        states[i].requested_percent = cfg->fans[i].reset_speed;
    }
}
