#ifndef ANS_CONFIG_PLATFORM_TYPES_H
#define ANS_CONFIG_PLATFORM_TYPES_H

#include "core/constants.h"

#include <stdbool.h>

/**
 * Default CoolBoost state for a model profile.
 *
 * CoolBoost is implemented through firmware turbo fan mode when the model
 * exposes fan-mode registers.
 */
struct coolboost_config
{
    bool default_enabled;
};

/**
 * EC register mapping for firmware fan modes.
 *
 * These values let the daemon switch between firmware auto, manual, and turbo
 * behavior without hardcoding one Nitro generation.
 */
struct fan_mode_config
{
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

/**
 * One firmware platform profile value.
 *
 * Profile names are user-facing; values are the EC payloads accepted by the
 * current model.
 */
struct platform_profile_entry
{
    char id[32];
    int value;
};

/**
 * Model-specific firmware platform profile mapping.
 *
 * The daemon uses this to report and apply quiet, balanced, and performance
 * profiles when the EC exposes them.
 */
struct platform_profile_config
{
    bool available;
    int reg;
    char default_profile[32];
    struct platform_profile_entry profiles[ANS_MAX_PLATFORM_PROFILES];
    int profile_len;
};

/**
 * AC and battery profile policy configured for automatic switching.
 *
 * The selected names reference platform profile entries and are applied when
 * power-source auto mode is enabled.
 */
struct power_source_profile_config
{
    bool auto_apply;
    char ac_profile[32];
    char battery_profile[32];
};

/**
 * EC keyboard backlight control and timeout capabilities.
 *
 * Some Nitro models expose keyboard brightness only through EC writes. Timeout
 * support is tracked separately because it is daemon-managed.
 */
struct keyboard_backlight_config
{
    bool available;
    int reg;
    int min_value;
    int max_value;
    bool timeout_supported;
    bool timeout_default_enabled;
    int timeout_seconds;
};

#endif
