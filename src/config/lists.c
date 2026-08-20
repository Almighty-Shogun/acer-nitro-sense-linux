#include "config/sections.h"

#include "config/parse.h"
#include "util/json.h"

#include <string.h>

int config_parse_allowed_dmi(const char *json, struct ans_config *cfg)
{
    const char *end;
    const char *p = json_find_array(json, "allowed_dmi_substrings", &end);

    if (!p)
        return 0;

    while (p < end && cfg->allowed_dmi_len < 8) {
        const char *q = strchr(p, '"');

        if (!q || q >= end)
            break;

        const char *r = strchr(q + 1, '"');

        if (!r || r > end)
            break;

        json_copy_slice(q + 1, r, cfg->allowed_dmi[cfg->allowed_dmi_len],
                        sizeof(cfg->allowed_dmi[0]));

        cfg->allowed_dmi_len++;
        p = r + 1;
    }

    return 0;
}

int config_parse_init_writes(const char *json, struct ans_config *cfg)
{
    const char *end;
    const char *p = json_find_array(json, "init_writes", &end);

    if (!p)
        return 0;

    while (p < end && cfg->init_write_len < ANS_MAX_WRITES) {
        const char *obj_end;
        const char *obj = json_next_object(p, end, &obj_end);
        char buf[512];
        struct ec_write_config *w = &cfg->init_writes[cfg->init_write_len];

        if (!obj)
            break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));

        if (!config_required_int_key(buf, "register", &w->reg) ||
            !config_required_int_key(buf, "value", &w->value) ||
            !config_required_int_key(buf, "reset_value", &w->reset_value) ||
            !config_byte_value_valid(w->reg) ||
            !config_byte_value_valid(w->value) ||
            !config_byte_value_valid(w->reset_value))
            return config_invalid(
                "init_writes entries require byte register/value/reset_value");

        cfg->init_write_len++;
        p = obj_end;
    }

    return 0;
}

int config_parse_presets(const char *json, struct ans_config *cfg)
{
    const char *end;
    const char *p = json_find_array(json, "presets", &end);

    if (!p)
        return 0;

    while (p < end && cfg->preset_len < ANS_MAX_PRESETS) {
        const char *obj_end;
        const char *obj = json_next_object(p, end, &obj_end);
        char buf[512];
        struct preset_config *preset = &cfg->presets[cfg->preset_len];

        if (!obj)
            break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));
        if (!config_required_string_key(buf, "id", preset->id,
                                        sizeof(preset->id)) ||
            !config_required_int_key(buf, "cpu", &preset->cpu) ||
            !config_required_int_key(buf, "gpu", &preset->gpu) ||
            !config_percent_value_valid(preset->cpu) ||
            !config_percent_value_valid(preset->gpu))
            return config_invalid(
                "preset entries require id and 1-100 cpu/gpu percentages");

        cfg->preset_len++;
        p = obj_end;
    }

    return 0;
}
