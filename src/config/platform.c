#include "config/sections.h"

#include "config/parse.h"
#include "util/json.h"

int config_parse_platform_profiles(const char *json, struct ans_config *cfg)
{
    const char *profiles = json_find_key(json, "platform_profiles");
    const char *end;
    const char *p;
    int reg;

    cfg->platform_profiles.available = false;
    cfg->platform_profiles.reg = 0;
    cfg->platform_profiles.default_profile[0] = '\0';
    cfg->platform_profiles.profile_len = 0;

    if (!profiles)
        return 0;

    if (!config_required_int_key(profiles, "register", &reg))
        return -1;
    if (!config_byte_value_valid(reg))
        return -1;

    cfg->platform_profiles.available = true;
    cfg->platform_profiles.reg = reg;
    if (!config_optional_string_key(profiles, "default_profile",
                                    cfg->platform_profiles.default_profile,
                                    sizeof(cfg->platform_profiles.default_profile)))
        return -1;

    p = json_find_array(profiles, "profiles", &end);
    if (!p)
        return 0;

    while (p < end &&
           cfg->platform_profiles.profile_len < ANS_MAX_PLATFORM_PROFILES) {
        const char *obj_end;
        const char *obj = json_next_object(p, end, &obj_end);
        char buf[512];
        struct platform_profile_entry *profile =
            &cfg->platform_profiles.profiles[cfg->platform_profiles.profile_len];

        if (!obj)
            break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));
        if (!config_required_string_key(buf, "id", profile->id,
                                        sizeof(profile->id)) ||
            !config_required_int_key(buf, "value", &profile->value) ||
            !config_byte_value_valid(profile->value))
            return -1;

        if (profile->id[0] != '\0')
            cfg->platform_profiles.profile_len++;

        p = obj_end;
    }

    return 0;
}

int config_parse_power_source_profiles(const char *json, struct ans_config *cfg)
{
    const char *profiles = json_find_key(json, "power_source_profiles");

    cfg->power_source_profiles.auto_apply = false;
    cfg->power_source_profiles.ac_profile[0] = '\0';
    cfg->power_source_profiles.battery_profile[0] = '\0';

    if (!profiles)
        return 0;

    if (!config_required_string_key(profiles, "ac",
                                    cfg->power_source_profiles.ac_profile,
                                    sizeof(cfg->power_source_profiles.ac_profile)) ||
        !config_required_string_key(profiles, "battery",
                                    cfg->power_source_profiles.battery_profile,
                                    sizeof(cfg->power_source_profiles.battery_profile)) ||
        !config_optional_bool_key(profiles, "auto_apply",
                                  &cfg->power_source_profiles.auto_apply))
        return -1;

    if (!cfg->platform_profiles.available ||
        !config_find_platform_profile(cfg,
                                      cfg->power_source_profiles.ac_profile) ||
        !config_find_platform_profile(cfg,
                                      cfg->power_source_profiles.battery_profile))
        return -1;

    return 0;
}

int config_parse_keyboard_backlight(const char *json, struct ans_config *cfg)
{
    const char *section = json_find_key(json, "keyboard_backlight");
    int reg;
    int min_value = 0;
    int max_value = 0;

    cfg->keyboard_backlight.available = false;
    cfg->keyboard_backlight.reg = -1;
    cfg->keyboard_backlight.min_value = 0;
    cfg->keyboard_backlight.max_value = 0;
    cfg->keyboard_backlight.timeout_supported = false;
    cfg->keyboard_backlight.timeout_default_enabled = false;
    cfg->keyboard_backlight.timeout_seconds = 30;

    if (!section)
        return 0;

    if (!config_required_int_key(section, "register", &reg) ||
        !config_required_int_key(section, "max_value", &max_value) ||
        !config_optional_int_key(section, "min_value", &min_value) ||
        !config_optional_bool_key(section, "timeout_supported",
                                  &cfg->keyboard_backlight.timeout_supported) ||
        !config_optional_bool_key(section, "timeout_default_enabled",
                                  &cfg->keyboard_backlight.timeout_default_enabled) ||
        !config_optional_int_key(section, "timeout_seconds",
                                 &cfg->keyboard_backlight.timeout_seconds))
        return -1;

    if (!config_byte_value_valid(reg) ||
        !config_byte_value_valid(min_value) ||
        !config_byte_value_valid(max_value) ||
        min_value >= max_value ||
        cfg->keyboard_backlight.timeout_seconds < 5 ||
        cfg->keyboard_backlight.timeout_seconds > 3600)
        return -1;

    cfg->keyboard_backlight.available = true;
    cfg->keyboard_backlight.reg = reg;
    cfg->keyboard_backlight.min_value = min_value;
    cfg->keyboard_backlight.max_value = max_value;
    return 0;
}
