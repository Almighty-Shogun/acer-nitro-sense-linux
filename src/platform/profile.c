#include "platform/control.h"

#include "ec/ec.h"
#include "config/config.h"

/**
 * Apply platform profile.
 *
 * Platform controls mirror Acer Sense features that are actually reachable on
 * Linux. Unsupported firmware surfaces should report clearly instead of
 * pretending to work.
 */
bool apply_platform_profile(struct ec_device* ec, const struct ans_config* cfg, const char* profile)
{
    if (!cfg->platform_profiles.available)
        return false;

    const struct platform_profile_entry* entry = config_find_platform_profile(cfg, profile);

    if (!entry)
        return false;

    return ec_write_byte(ec, cfg->platform_profiles.reg, entry->value) == 0;
}

/**
 * Compute value name.
 *
 * Platform controls mirror Acer Sense features that are actually reachable on
 * Linux. Unsupported firmware surfaces should report clearly instead of
 * pretending to work.
 */
const char* platform_profile_value_name(const struct ans_config* cfg, const int value)
{
    for (int i = 0; i < cfg->platform_profiles.profile_len; i++)
    {
        if (cfg->platform_profiles.profiles[i].value == value)
            return cfg->platform_profiles.profiles[i].id;
    }

    return "unknown";
}

/**
 * Read platform profile.
 *
 * Platform controls mirror Acer Sense features that are actually reachable on
 * Linux. Unsupported firmware surfaces should report clearly instead of
 * pretending to work.
 */
bool read_platform_profile(struct ec_device* ec, const struct ans_config* cfg, int* value)
{
    if (!cfg->platform_profiles.available)
        return false;

    *value = ec_read_byte(ec, cfg->platform_profiles.reg);

    return *value >= 0;
}
