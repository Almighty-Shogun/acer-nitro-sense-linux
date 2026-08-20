#include "selftest/fixture.h"

#include <stdio.h>
#include <string.h>

static void init_core_config(struct ans_config *cfg)
{
    snprintf(cfg->model, sizeof(cfg->model), "Acer Nitro Self Test");
    snprintf(cfg->default_preset, sizeof(cfg->default_preset), "auto");
    snprintf(cfg->ec_path, sizeof(cfg->ec_path), "fake");
    cfg->poll_interval_ms = 100;
    cfg->critical_temperature_c = 90;
    cfg->read_words = true;
    cfg->coolboost.default_enabled = false;
}

static void init_safety_config(struct safety_config *safety)
{
    safety->min_speed_percent = 35;
    safety->min_speed_temperature_c = 60;
    safety->critical_speed_percent = 100;
    safety->critical_full_speed = true;
    safety->critical_step_percent = 20;
    safety->critical_consecutive_samples = 1;
    safety->critical_release_temperature_c = 85;
    safety->auto_ramp_up_percent = 12;
    safety->auto_ramp_bypass_temperature_c = 0;
    safety->missing_temperature_speed_percent = 60;
    safety->max_ec_read_failures = 3;
    safety->max_ec_write_failures = 3;
}

static void init_fan_mode_config(struct fan_mode_config *fan_modes)
{
    fan_modes->available = true;
    fan_modes->cpu_reg = 0x22;
    fan_modes->cpu_auto_value = 0x04;
    fan_modes->cpu_manual_value = 0x0c;
    fan_modes->cpu_turbo_value = 0x08;
    fan_modes->gpu_reg = 0x21;
    fan_modes->gpu_auto_value = 0x10;
    fan_modes->gpu_manual_value = 0x30;
    fan_modes->gpu_turbo_value = 0x20;
}

static void init_platform_profiles(struct platform_profile_config *profiles)
{
    profiles->available = true;
    profiles->reg = 0x2c;
    snprintf(profiles->default_profile, sizeof(profiles->default_profile),
             "balanced");
    profiles->profile_len = 3;

    snprintf(profiles->profiles[0].id, sizeof(profiles->profiles[0].id),
             "quiet");
    profiles->profiles[0].value = 0x00;
    snprintf(profiles->profiles[1].id, sizeof(profiles->profiles[1].id),
             "balanced");
    profiles->profiles[1].value = 0x01;
    snprintf(profiles->profiles[2].id, sizeof(profiles->profiles[2].id),
             "performance");
    profiles->profiles[2].value = 0x04;
}

static void init_power_source_profiles(struct power_source_profile_config *profiles)
{
    profiles->auto_apply = false;
    snprintf(profiles->ac_profile, sizeof(profiles->ac_profile), "balanced");
    snprintf(profiles->battery_profile, sizeof(profiles->battery_profile),
             "quiet");
}

static void init_keyboard_backlight_config(struct keyboard_backlight_config *backlight)
{
    backlight->available = true;
    backlight->reg = 0x31;
    backlight->min_value = 0;
    backlight->max_value = 4;
    backlight->timeout_supported = true;
    backlight->timeout_default_enabled = false;
    backlight->timeout_seconds = 30;
}

static void init_cpu_fan(struct fan_config *fan)
{
    snprintf(fan->id, sizeof(fan->id), "cpu");
    snprintf(fan->name, sizeof(fan->name), "CPU fan");
    snprintf(fan->sensor_group, sizeof(fan->sensor_group), "cpu");
    snprintf(fan->control_sensor_group, sizeof(fan->control_sensor_group), "max");
    fan->read_register = 0x13;
    fan->write_register = 0x37;
    fan->temperature_register = -1;
    fan->control_temperature_register = -1;
    fan->write_min = 0;
    fan->write_max = 100;
    fan->reset_speed = 50;
    fan->missing_temperature_speed_percent = 62;
    fan->curve_len = 2;
    fan->curve[0] = (struct threshold){.up = 0, .down = 0, .speed = 30};
    fan->curve[1] = (struct threshold){.up = 60, .down = 55, .speed = 55};
}

static void init_gpu_fan(struct fan_config *fan)
{
    snprintf(fan->id, sizeof(fan->id), "gpu");
    snprintf(fan->name, sizeof(fan->name), "GPU fan");
    snprintf(fan->sensor_group, sizeof(fan->sensor_group), "gpu");
    snprintf(fan->control_sensor_group, sizeof(fan->control_sensor_group), "max");
    fan->read_register = 0x15;
    fan->write_register = 0x3a;
    fan->temperature_register = -1;
    fan->control_temperature_register = -1;
    fan->write_min = 0;
    fan->write_max = 100;
    fan->reset_speed = 50;
    fan->curve_len = 2;
    fan->curve[0] = (struct threshold){.up = 0, .down = 0, .speed = 25};
    fan->curve[1] = (struct threshold){.up = 60, .down = 55, .speed = 55};
}

static void init_fans(struct ans_config *cfg)
{
    cfg->fan_len = 2;
    init_cpu_fan(&cfg->fans[0]);
    init_gpu_fan(&cfg->fans[1]);
}

static void init_presets(struct ans_config *cfg)
{
    cfg->preset_len = 2;
    snprintf(cfg->presets[0].id, sizeof(cfg->presets[0].id), "balanced");
    cfg->presets[0].cpu = 50;
    cfg->presets[0].gpu = 45;
    snprintf(cfg->presets[1].id, sizeof(cfg->presets[1].id), "max");
    cfg->presets[1].cpu = 100;
    cfg->presets[1].gpu = 100;
}

void init_self_test_config(struct ans_config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    init_core_config(cfg);
    init_safety_config(&cfg->safety);
    init_fan_mode_config(&cfg->fan_modes);
    init_platform_profiles(&cfg->platform_profiles);
    init_power_source_profiles(&cfg->power_source_profiles);
    init_keyboard_backlight_config(&cfg->keyboard_backlight);
    init_fans(cfg);
    init_presets(cfg);
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
