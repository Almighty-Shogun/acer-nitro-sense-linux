#include "platform/power_source.h"

#include "platform/control.h"
#include "util/file.h"
#include "util/string.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *power_source_name(const enum power_source_state source)
{
    switch (source) {
    case POWER_SOURCE_AC:
        return "ac";
    case POWER_SOURCE_BATTERY:
        return "battery";
    case POWER_SOURCE_UNKNOWN:
    default:
        return "unknown";
    }
}

static enum power_source_state fake_power_source(void)
{
    const char *fake = getenv("ANS_FAKE_POWER_SOURCE");

    if (!fake || fake[0] == '\0')
        return POWER_SOURCE_UNKNOWN;
    if (strcmp(fake, "ac") == 0)
        return POWER_SOURCE_AC;
    if (strcmp(fake, "battery") == 0)
        return POWER_SOURCE_BATTERY;

    return POWER_SOURCE_UNKNOWN;
}

static bool read_trimmed_file(const char *path, char *out, const size_t out_len)
{
    char *text = read_text_file(path, 4096);

    if (!text)
        return false;

    trim_ascii(text);
    string_copy(out, out_len, text);
    free(text);
    return true;
}

enum power_source_state read_power_source(void)
{
    enum power_source_state fake = fake_power_source();
    DIR *dir;
    struct dirent *entry;
    bool has_battery = false;
    bool has_mains = false;

    if (fake != POWER_SOURCE_UNKNOWN)
        return fake;

    dir = opendir("/sys/class/power_supply");
    if (!dir)
        return POWER_SOURCE_UNKNOWN;

    while ((entry = readdir(dir))) {
        char path[512];
        char type_path[1024];
        char online_path[1024];
        char type[64];
        char online[16];

        if (entry->d_name[0] == '.')
            continue;

        snprintf(path, sizeof(path), "/sys/class/power_supply/%s", entry->d_name);
        snprintf(type_path, sizeof(type_path), "%s/type", path);
        if (!read_trimmed_file(type_path, type, sizeof(type)))
            continue;

        if (strcmp(type, "Battery") == 0) {
            has_battery = true;
            continue;
        }

        if (strcmp(type, "Mains") != 0 && strcmp(type, "USB") != 0 &&
            strcmp(type, "USB-C") != 0 && strcmp(type, "USB_PD") != 0)
            continue;

        has_mains = true;
        snprintf(online_path, sizeof(online_path), "%s/online", path);
        if (read_trimmed_file(online_path, online, sizeof(online)) &&
            strcmp(online, "1") == 0) {
            closedir(dir);
            return POWER_SOURCE_AC;
        }
    }

    closedir(dir);

    if (has_battery)
        return POWER_SOURCE_BATTERY;
    if (has_mains)
        return POWER_SOURCE_AC;

    return POWER_SOURCE_UNKNOWN;
}

const char *power_source_profile_for(const struct ans_config *cfg,
                                     const enum power_source_state source)
{
    if (!power_source_profile_policy_available(cfg))
        return NULL;

    if (source == POWER_SOURCE_AC)
        return cfg->power_source_profiles.ac_profile;
    if (source == POWER_SOURCE_BATTERY)
        return cfg->power_source_profiles.battery_profile;

    return NULL;
}

bool power_source_profile_policy_available(const struct ans_config *cfg)
{
    return cfg->platform_profiles.available &&
           cfg->power_source_profiles.ac_profile[0] != '\0' &&
           cfg->power_source_profiles.battery_profile[0] != '\0';
}

bool apply_power_source_profile(struct ec_device *ec, const struct ans_config *cfg,
                                const enum power_source_state source)
{
    const char *profile = power_source_profile_for(cfg, source);

    if (!profile)
        return false;

    return apply_platform_profile(ec, cfg, profile);
}
