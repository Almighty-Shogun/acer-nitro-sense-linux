#ifndef ANS_CONFIG_TYPES_H
#define ANS_CONFIG_TYPES_H

#include "core/constants.h"

#include <stdbool.h>

struct threshold {
    int up;
    int down;
    int speed;
};

struct fan_config {
    char id[16];
    char name[64];
    char sensor_group[16];
    char control_sensor_group[16];
    char sensor_power_control[8];
    bool keep_awake;
    int read_register;
    int write_register;
    int temperature_register;
    int control_temperature_register;
    int read_min;
    int read_max;
    int write_min;
    int write_max;
    int reset_speed;
    int missing_temperature_speed_percent;
    struct threshold curve[ANS_MAX_THRESHOLDS];
    int curve_len;
};

struct ec_write_config {
    int reg;
    int value;
    int reset_value;
};

struct preset_config {
    char id[32];
    int cpu;
    int gpu;
};

struct safety_config {
    int min_speed_percent;
    int min_speed_temperature_c;
    int critical_speed_percent;
    bool critical_full_speed;
    int critical_step_percent;
    int critical_consecutive_samples;
    int critical_release_temperature_c;
    int auto_ramp_up_percent;
    int auto_ramp_bypass_temperature_c;
    int missing_temperature_speed_percent;
    int max_ec_read_failures;
    int max_ec_write_failures;
};

struct coolboost_config {
    bool default_enabled;
};

struct fan_mode_config {
    bool available;
    int cpu_reg;
    int gpu_reg;
    int cpu_auto_value;
    int cpu_manual_value;
    int cpu_turbo_value;
    int gpu_auto_value;
    int gpu_manual_value;
    int gpu_turbo_value;
};

struct platform_profile_entry {
    char id[32];
    int value;
};

struct platform_profile_config {
    bool available;
    int reg;
    char default_profile[32];
    struct platform_profile_entry profiles[ANS_MAX_PLATFORM_PROFILES];
    int profile_len;
};

struct power_source_profile_config {
    bool auto_apply;
    char ac_profile[32];
    char battery_profile[32];
};

struct keyboard_backlight_config {
    bool available;
    int reg;
    int min_value;
    int max_value;
    bool timeout_supported;
    bool timeout_default_enabled;
    int timeout_seconds;
};

struct ans_config {
    char model[96];
    char default_preset[32];
    char ec_path[256];
    char allowed_dmi[8][64];
    int allowed_dmi_len;
    int poll_interval_ms;
    int critical_temperature_c;
    bool read_words;
    struct safety_config safety;
    struct coolboost_config coolboost;
    struct fan_mode_config fan_modes;
    struct platform_profile_config platform_profiles;
    struct power_source_profile_config power_source_profiles;
    struct keyboard_backlight_config keyboard_backlight;
    struct ec_write_config init_writes[ANS_MAX_WRITES];
    int init_write_len;
    struct fan_config fans[ANS_MAX_FANS];
    int fan_len;
    struct preset_config presets[ANS_MAX_PRESETS];
    int preset_len;
};

#endif
