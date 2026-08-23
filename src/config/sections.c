#include "config/sections.h"

#include "util/json.h"
#include "config/parse.h"

/**
 * Parse fan mode register locations.
 *
 * The fan mode section controls both CPU and GPU fan controller modes, so both
 * EC registers are required whenever the section is present.
 */
static bool parse_fan_mode_registers(const char* fan_modes, int* cpu_reg, int* gpu_reg)
{
    const bool has_cpu_register = config_required_int_key(fan_modes, "cpu_register", cpu_reg);
    const bool has_gpu_register = config_required_int_key(fan_modes, "gpu_register", gpu_reg);

    if (!has_cpu_register || !has_gpu_register)
        return false;

    return config_byte_value_valid(*cpu_reg) && config_byte_value_valid(*gpu_reg);
}

/**
 * Seed firmware fan mode defaults.
 *
 * These values match the Nitro firmware mode bytes used by the current model
 * unless the JSON model overrides them explicitly.
 */
static void set_default_fan_mode_values(struct ans_config* cfg)
{
    cfg->fan_modes.cpu_auto_value = 4;
    cfg->fan_modes.cpu_manual_value = 12;
    cfg->fan_modes.cpu_turbo_value = 8;
    cfg->fan_modes.gpu_auto_value = 16;
    cfg->fan_modes.gpu_manual_value = 48;
    cfg->fan_modes.gpu_turbo_value = 32;
}

/**
 * Parse fan mode byte overrides.
 *
 * Mode values are optional so new model profiles only need to specify them
 * when their firmware differs from the known Nitro defaults.
 */
static bool parse_fan_mode_values(const char* fan_modes, struct ans_config* cfg)
{
    const bool cpu_auto_valid = config_optional_int_key(
        fan_modes,
        "cpu_auto_value",
        &cfg->fan_modes.cpu_auto_value
    );

    const bool cpu_manual_valid = config_optional_int_key(
        fan_modes,
        "cpu_manual_value",
        &cfg->fan_modes.cpu_manual_value
    );

    const bool cpu_turbo_valid = config_optional_int_key(
        fan_modes,
        "cpu_turbo_value",
        &cfg->fan_modes.cpu_turbo_value
    );

    const bool gpu_auto_valid = config_optional_int_key(
        fan_modes,
        "gpu_auto_value",
        &cfg->fan_modes.gpu_auto_value
    );

    const bool gpu_manual_valid = config_optional_int_key(
        fan_modes,
        "gpu_manual_value",
        &cfg->fan_modes.gpu_manual_value
    );

    const bool gpu_turbo_valid = config_optional_int_key(
        fan_modes,
        "gpu_turbo_value",
        &cfg->fan_modes.gpu_turbo_value
    );

    return cpu_auto_valid
           && cpu_manual_valid
           && cpu_turbo_valid
           && gpu_auto_valid
           && gpu_manual_valid
           && gpu_turbo_valid;
}

/**
 * Validate fan mode byte values.
 *
 * Fan mode values are written directly to EC registers, so every parsed value
 * must fit into a single byte.
 */
static bool fan_mode_values_valid(const struct ans_config* cfg)
{
    return config_byte_value_valid(cfg->fan_modes.cpu_auto_value)
           && config_byte_value_valid(cfg->fan_modes.cpu_manual_value)
           && config_byte_value_valid(cfg->fan_modes.cpu_turbo_value)
           && config_byte_value_valid(cfg->fan_modes.gpu_auto_value)
           && config_byte_value_valid(cfg->fan_modes.gpu_manual_value)
           && config_byte_value_valid(cfg->fan_modes.gpu_turbo_value);
}

/**
 * Parse modes.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
int config_parse_fan_modes(const char* json, struct ans_config* cfg)
{
    const char* fan_modes = json_find_key(json, "fan_modes");

    int cpu_reg, gpu_reg;

    cfg->fan_modes.available = false;
    cfg->fan_modes.cpu_reg = 0;
    cfg->fan_modes.gpu_reg = 0;
    cfg->fan_modes.cpu_auto_value = 0;
    cfg->fan_modes.cpu_manual_value = 0;
    cfg->fan_modes.cpu_turbo_value = 0;
    cfg->fan_modes.gpu_auto_value = 0;
    cfg->fan_modes.gpu_manual_value = 0;
    cfg->fan_modes.gpu_turbo_value = 0;

    if (!fan_modes)
        return 0;

    if (!parse_fan_mode_registers(fan_modes, &cpu_reg, &gpu_reg))
        return -1;

    cfg->fan_modes.available = true;
    cfg->fan_modes.cpu_reg = cpu_reg;
    cfg->fan_modes.gpu_reg = gpu_reg;

    set_default_fan_mode_values(cfg);

    if (!parse_fan_mode_values(fan_modes, cfg))
        return -1;

    if (!fan_mode_values_valid(cfg))
        return -1;

    return 0;
}
