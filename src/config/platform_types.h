#ifndef ANS_CONFIG_PLATFORM_TYPES_H
#define ANS_CONFIG_PLATFORM_TYPES_H

#include "core/constants.h"

#include <stdbool.h>

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

#endif
