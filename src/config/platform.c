#include "config/sections.h"

#include "util/json.h"
#include "config/parse.h"
#include "config/config.h"

/**
 * Parse one platform profile entry.
 *
 * Profile ids are command-visible names and values are written directly to the
 * platform profile EC register.
 */
static bool parse_platform_profile_entry(const char* profile_json, struct platform_profile_entry* profile)
{
    const bool has_value = config_required_int_key(profile_json, "value", &profile->value);
    const bool has_id = config_required_string_key(profile_json, "id", profile->id, sizeof(profile->id));

    const bool value_valid = has_value && config_byte_value_valid(profile->value);

    return has_id && value_valid;
}

/**
 * Parse platform profile defaults.
 *
 * The profile register is mandatory when the section exists. The default
 * profile name is optional and stays empty when omitted.
 */
static bool parse_platform_profile_header(const char* profiles, struct ans_config* cfg, int* reg)
{
    const bool has_register = config_required_int_key(profiles, "register", reg);

    const bool register_valid = has_register && config_byte_value_valid(*reg);

    const bool default_profile_valid = config_optional_string_key(
        profiles,
        "default_profile",
        cfg->platform_profiles.default_profile,
        sizeof(cfg->platform_profiles.default_profile)
    );

    return register_valid && default_profile_valid;
}

/**
 * Parse platform profile mappings.
 *
 * Platform profiles map user-facing names to byte values written through the
 * model's platform profile EC register.
 */
int config_parse_platform_profiles(const char* json, struct ans_config* cfg)
{
    const char* profiles = json_find_key(json, "platform_profiles");

    int reg;
    const char* end;

    cfg->platform_profiles.available = false;
    cfg->platform_profiles.reg = 0;
    cfg->platform_profiles.default_profile[0] = '\0';
    cfg->platform_profiles.profile_len = 0;

    if (!profiles)
        return 0;

    if (!parse_platform_profile_header(profiles, cfg, &reg))
        return -1;

    cfg->platform_profiles.available = true;
    cfg->platform_profiles.reg = reg;

    const char* p = json_find_array(profiles, "profiles", &end);

    if (!p)
        return 0;

    while (p < end && cfg->platform_profiles.profile_len < ANS_MAX_PLATFORM_PROFILES)
    {
        const char* obj_end;
        const char* obj = json_next_object(p, end, &obj_end);

        char buf[512];
        struct platform_profile_entry* profile = &cfg->platform_profiles.profiles[cfg->platform_profiles.profile_len];

        if (!obj) break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));

        if (!parse_platform_profile_entry(buf, profile))
            return -1;

        if (profile->id[0] != '\0')
            cfg->platform_profiles.profile_len++;

        p = obj_end;
    }

    return 0;
}

/**
 * Parse power-source profile names.
 *
 * Both AC and battery profiles are required when the section exists. The
 * auto-apply flag is optional and defaults to false.
 */
static bool parse_power_source_profile_fields(const char* profiles, struct ans_config* cfg)
{
    const bool has_ac_profile = config_required_string_key(
        profiles,
        "ac",
        cfg->power_source_profiles.ac_profile,
        sizeof(cfg->power_source_profiles.ac_profile)
    );

    const bool has_battery_profile = config_required_string_key(
        profiles,
        "battery",
        cfg->power_source_profiles.battery_profile,
        sizeof(cfg->power_source_profiles.battery_profile)
    );

    const bool auto_apply_valid = config_optional_bool_key(
        profiles,
        "auto_apply",
        &cfg->power_source_profiles.auto_apply
    );

    return has_ac_profile && has_battery_profile && auto_apply_valid;
}

/**
 * Validate power-source profile references.
 *
 * Power-source policy depends on platform profiles, so both referenced names
 * must exist in the parsed platform profile table.
 */
static bool power_source_profiles_refer_to_known_profiles(const struct ans_config* cfg)
{
    const bool platform_profiles_available = cfg->platform_profiles.available;
    const bool ac_profile_known = config_find_platform_profile(cfg, cfg->power_source_profiles.ac_profile) != NULL;
    const bool battery_profile_known = config_find_platform_profile(cfg, cfg->power_source_profiles.battery_profile) != NULL;

    return platform_profiles_available && ac_profile_known && battery_profile_known;
}

/**
 * Parse AC and battery profile policy.
 *
 * The section is optional. When present, it links AC and battery state to
 * already parsed platform profiles.
 */
int config_parse_power_source_profiles(const char* json, struct ans_config* cfg)
{
    const char* profiles = json_find_key(json, "power_source_profiles");

    cfg->power_source_profiles.auto_apply = false;
    cfg->power_source_profiles.ac_profile[0] = '\0';
    cfg->power_source_profiles.battery_profile[0] = '\0';

    if (!profiles)
        return 0;

    if (!parse_power_source_profile_fields(profiles, cfg))
        return -1;

    if (!power_source_profiles_refer_to_known_profiles(cfg))
        return -1;

    return 0;
}

/**
 * Parse keyboard backlight fields.
 *
 * The register and max value are mandatory. Minimum value and timeout behavior
 * are optional and start from caller-provided defaults.
 */
static bool parse_keyboard_backlight_fields(const char* section, struct ans_config* cfg, int* reg, int* min_value, int* max_value)
{
    const bool has_register = config_required_int_key(section, "register", reg);
    const bool has_max_value = config_required_int_key(section, "max_value", max_value);
    const bool has_min_value = config_optional_int_key(section, "min_value", min_value);

    const bool timeout_supported_valid = config_optional_bool_key(
        section,
        "timeout_supported",
        &cfg->keyboard_backlight.timeout_supported
    );

    const bool timeout_default_valid = config_optional_bool_key(
        section,
        "timeout_default_enabled",
        &cfg->keyboard_backlight.timeout_default_enabled
    );

    const bool timeout_seconds_valid = config_optional_int_key(
        section,
        "timeout_seconds",
        &cfg->keyboard_backlight.timeout_seconds
    );

    return has_register
           && has_max_value
           && has_min_value
           && timeout_supported_valid
           && timeout_default_valid
           && timeout_seconds_valid;
}

/**
 * Validate keyboard backlight ranges.
 *
 * The brightness register and values must fit byte writes, and timeout values
 * are bounded to avoid accidental instant-off or unreasonably long waits.
 */
static bool keyboard_backlight_values_valid(const struct ans_config* cfg, const int reg, const int min_value, const int max_value)
{
    const bool byte_values_valid = config_byte_value_valid(reg)
                                   && config_byte_value_valid(min_value)
                                   && config_byte_value_valid(max_value);

    const bool brightness_range_valid = min_value < max_value;
    const bool timeout_seconds_valid = cfg->keyboard_backlight.timeout_seconds >= 5 && cfg->keyboard_backlight.timeout_seconds <= 3600;

    return byte_values_valid && brightness_range_valid && timeout_seconds_valid;
}

/**
 * Parse keyboard backlight capabilities.
 *
 * Keyboard backlight support is optional. A present section enables EC-backed
 * brightness control once all register and timeout values validate.
 */
int config_parse_keyboard_backlight(const char* json, struct ans_config* cfg)
{
    const char* section = json_find_key(json, "keyboard_backlight");

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

    if (!parse_keyboard_backlight_fields(section, cfg, &reg, &min_value, &max_value))
        return -1;

    if (!keyboard_backlight_values_valid(cfg, reg, min_value, max_value))
        return -1;

    cfg->keyboard_backlight.available = true;
    cfg->keyboard_backlight.reg = reg;
    cfg->keyboard_backlight.min_value = min_value;
    cfg->keyboard_backlight.max_value = max_value;

    return 0;
}
