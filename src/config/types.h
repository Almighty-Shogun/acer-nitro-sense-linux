#ifndef ANS_CONFIG_TYPES_H
#define ANS_CONFIG_TYPES_H

#include "config/fan_types.h"
#include "config/platform_types.h"
#include "config/safety_types.h"
#include "core/constants.h"

#include <stdbool.h>

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
