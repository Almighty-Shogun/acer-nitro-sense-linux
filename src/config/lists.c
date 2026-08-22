#include "config/sections.h"

#include "util/json.h"
#include "config/parse.h"

#include <string.h>

/**
 * Parse allowed DMI model names.
 *
 * Missing DMI allow-lists are accepted for development profiles. Present lists
 * are copied into fixed-size config storage for later hardware checks.
 */
int config_parse_allowed_dmi(const char* json, struct ans_config* cfg)
{
    const char* end;
    const char* p = json_find_array(json, "allowed_dmi_substrings", &end);

    if (!p)
        return 0;

    while (p < end && cfg->allowed_dmi_len < 8)
    {
        const char* q = strchr(p, '"');

        if (!q || q >= end) break;

        const char* r = strchr(q + 1, '"');

        if (!r || r > end) break;

        json_copy_slice(q + 1, r, cfg->allowed_dmi[cfg->allowed_dmi_len], sizeof(cfg->allowed_dmi[0]));

        cfg->allowed_dmi_len++;

        p = r + 1;
    }

    return 0;
}

/**
 * Parse one EC initialization write.
 *
 * Init writes touch EC registers during daemon startup and shutdown, so all
 * three values must be present and byte-sized.
 */
static int parse_init_write(const char* write_json, struct ec_write_config* write)
{
    const bool has_register = config_required_int_key(write_json, "register", &write->reg);
    const bool has_value = config_required_int_key(write_json, "value", &write->value);
    const bool has_reset_value = config_required_int_key(write_json, "reset_value", &write->reset_value);

    const bool fields_present = has_register && has_value && has_reset_value;

    const bool byte_values_valid = config_byte_value_valid(write->reg)
        && config_byte_value_valid(write->value)
        && config_byte_value_valid(write->reset_value);

    return fields_present && byte_values_valid
               ? 0
               : config_invalid("init_writes entries require byte register/value/reset_value");
}

/**
 * Parse EC initialization writes.
 *
 * The section is optional. When present, every entry must define the startup
 * write and the reset value used when the daemon stops.
 */
int config_parse_init_writes(const char* json, struct ans_config* cfg)
{
    const char* end;
    const char* p = json_find_array(json, "init_writes", &end);

    if (!p)
        return 0;

    while (p < end && cfg->init_write_len < ANS_MAX_WRITES)
    {
        const char* obj_end;
        const char* obj = json_next_object(p, end, &obj_end);

        char buf[512];
        struct ec_write_config* write = &cfg->init_writes[cfg->init_write_len];

        if (!obj) break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));

        if (parse_init_write(buf, write) < 0)
            return -1;

        cfg->init_write_len++;
        p = obj_end;
    }

    return 0;
}

/**
 * Parse one fan-speed preset.
 *
 * Presets must have a command-visible id and valid CPU/GPU fan percentages.
 */
static int parse_preset(const char* preset_json, struct preset_config* preset)
{
    const bool has_cpu = config_required_int_key(preset_json, "cpu", &preset->cpu);
    const bool has_gpu = config_required_int_key(preset_json, "gpu", &preset->gpu);
    const bool has_id = config_required_string_key(preset_json, "id", preset->id, sizeof(preset->id));

    const bool fields_present = has_id && has_cpu && has_gpu;

    const bool percentages_valid = config_percent_value_valid(preset->cpu) && config_percent_value_valid(preset->gpu);

    return fields_present && percentages_valid
               ? 0
               : config_invalid("preset entries require id and 1-100 cpu/gpu percentages");
}

/**
 * Parse named fan presets.
 *
 * The section is optional. Present entries become user-facing shortcuts for
 * setting CPU and GPU fan percentages together.
 */
int config_parse_presets(const char* json, struct ans_config* cfg)
{
    const char* end;
    const char* p = json_find_array(json, "presets", &end);

    if (!p)
        return 0;

    while (p < end && cfg->preset_len < ANS_MAX_PRESETS)
    {
        const char* obj_end;
        const char* obj = json_next_object(p, end, &obj_end);

        char buf[512];
        struct preset_config* preset = &cfg->presets[cfg->preset_len];

        if (!obj) break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));

        if (parse_preset(buf, preset) < 0)
            return -1;

        cfg->preset_len++;
        p = obj_end;
    }

    return 0;
}
