#include "platform/control.h"

bool apply_platform_profile(struct ec_device *ec, const struct ans_config *cfg,
                            const char *profile)
{
    const struct platform_profile_entry *entry;

    if (!cfg->platform_profiles.available)
        return false;

    entry = config_find_platform_profile(cfg, profile);
    if (!entry)
        return false;

    return ec_write_byte(ec, cfg->platform_profiles.reg, entry->value) == 0;
}

const char *platform_profile_value_name(const struct ans_config *cfg,
                                        const int value)
{
    for (int i = 0; i < cfg->platform_profiles.profile_len; i++) {
        if (cfg->platform_profiles.profiles[i].value == value)
            return cfg->platform_profiles.profiles[i].id;
    }

    return "unknown";
}

bool read_platform_profile(struct ec_device *ec, const struct ans_config *cfg,
                           int *value)
{
    if (!cfg->platform_profiles.available)
        return false;

    *value = ec_read_byte(ec, cfg->platform_profiles.reg);

    return *value >= 0;
}
