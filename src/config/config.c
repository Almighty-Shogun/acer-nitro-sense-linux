#include "ans.h"
#include "config/parse.h"
#include "config/sections.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int config_load(const char *path, struct ans_config *cfg)
{
    char *json = read_text_file(path, 128 * 1024);

    if (!json)
        return -1;

    memset(cfg, 0, sizeof(*cfg));
    snprintf(cfg->ec_path, sizeof(cfg->ec_path), "/sys/kernel/debug/ec/ec0/io");
    if (!config_required_string_key(json, "model", cfg->model,
                                    sizeof(cfg->model)) ||
        !config_required_string_key(json, "default_preset", cfg->default_preset,
                                    sizeof(cfg->default_preset)) ||
        !config_optional_string_key(json, "path", cfg->ec_path,
                                    sizeof(cfg->ec_path))) {
        config_invalid("model/default_preset/path fields are invalid");
        free(json);
        return -1;
    }

    cfg->poll_interval_ms = 3000;
    cfg->critical_temperature_c = 90;
    cfg->read_words = true;
    if (!config_optional_int_key(json, "poll_interval_ms",
                                 &cfg->poll_interval_ms) ||
        cfg->poll_interval_ms < 100 || cfg->poll_interval_ms > 60000) {
        config_invalid("poll_interval_ms must be 100-60000");
        free(json);
        return -1;
    }
    if (!config_optional_int_key(json, "critical_temperature_c",
                                 &cfg->critical_temperature_c) ||
        cfg->critical_temperature_c < 1 || cfg->critical_temperature_c > 130) {
        config_invalid("critical_temperature_c must be 1-130");
        free(json);
        return -1;
    }
    if (!config_optional_bool_key(json, "read_words", &cfg->read_words)) {
        config_invalid("read_words must be true or false");
        free(json);
        return -1;
    }
    if (config_parse_safety(json, cfg) < 0) {
        config_invalid("safety section is invalid");
        free(json);
        return -1;
    }
    cfg->coolboost.default_enabled = false;
    if (config_parse_fan_modes(json, cfg) < 0) {
        config_invalid("fan_modes section is invalid");
        free(json);
        return -1;
    }
    if (config_parse_platform_profiles(json, cfg) < 0) {
        config_invalid("platform_profiles section is invalid");
        free(json);
        return -1;
    }
    if (config_parse_power_source_profiles(json, cfg) < 0) {
        config_invalid("power_source_profiles section is invalid");
        free(json);
        return -1;
    }
    if (config_parse_keyboard_backlight(json, cfg) < 0) {
        config_invalid("keyboard_backlight section is invalid");
        free(json);
        return -1;
    }

    config_parse_allowed_dmi(json, cfg);
    if (config_parse_init_writes(json, cfg) < 0) {
        free(json);
        return -1;
    }

    if (config_parse_fans(json, cfg) < 0) {
        config_invalid("fans section is invalid");
        free(json);

        return -1;
    }

    if (config_parse_presets(json, cfg) < 0) {
        config_invalid("presets section is invalid");
        free(json);
        return -1;
    }

    free(json);

    return 0;
}

const struct preset_config *config_find_preset(const struct ans_config *cfg, const char *id)
{
    for (int i = 0; i < cfg->preset_len; i++) {
        if (strcmp(cfg->presets[i].id, id) == 0)
            return &cfg->presets[i];
    }

    return NULL;
}

const struct platform_profile_entry *config_find_platform_profile(const struct ans_config *cfg, const char *id)
{
    for (int i = 0; i < cfg->platform_profiles.profile_len; i++) {
        if (strcmp(cfg->platform_profiles.profiles[i].id, id) == 0)
            return &cfg->platform_profiles.profiles[i];
    }

    return NULL;
}

const struct fan_config *config_find_fan(const struct ans_config *cfg, const char *id)
{
    for (int i = 0; i < cfg->fan_len; i++) {
        if (strcmp(cfg->fans[i].id, id) == 0)
            return &cfg->fans[i];
    }

    return NULL;
}
