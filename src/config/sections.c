#include "config/sections.h"

#include "config/parse.h"
#include "util/json.h"

int config_parse_fan_modes(const char *json, struct ans_config *cfg)
{
    const char *fan_modes = json_find_key(json, "fan_modes");
    int cpu_reg;
    int gpu_reg;

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

    if (!config_required_int_key(fan_modes, "cpu_register", &cpu_reg) ||
        !config_required_int_key(fan_modes, "gpu_register", &gpu_reg))
        return -1;
    if (!config_byte_value_valid(cpu_reg) || !config_byte_value_valid(gpu_reg))
        return -1;

    cfg->fan_modes.available = true;
    cfg->fan_modes.cpu_reg = cpu_reg;
    cfg->fan_modes.gpu_reg = gpu_reg;
    cfg->fan_modes.cpu_auto_value = 4;
    cfg->fan_modes.cpu_manual_value = 12;
    cfg->fan_modes.cpu_turbo_value = 8;
    cfg->fan_modes.gpu_auto_value = 16;
    cfg->fan_modes.gpu_manual_value = 48;
    cfg->fan_modes.gpu_turbo_value = 32;
    if (!config_optional_int_key(fan_modes, "cpu_auto_value",
                                 &cfg->fan_modes.cpu_auto_value) ||
        !config_optional_int_key(fan_modes, "cpu_manual_value",
                                 &cfg->fan_modes.cpu_manual_value) ||
        !config_optional_int_key(fan_modes, "cpu_turbo_value",
                                 &cfg->fan_modes.cpu_turbo_value) ||
        !config_optional_int_key(fan_modes, "gpu_auto_value",
                                 &cfg->fan_modes.gpu_auto_value) ||
        !config_optional_int_key(fan_modes, "gpu_manual_value",
                                 &cfg->fan_modes.gpu_manual_value) ||
        !config_optional_int_key(fan_modes, "gpu_turbo_value",
                                 &cfg->fan_modes.gpu_turbo_value))
        return -1;

    if (!config_byte_value_valid(cfg->fan_modes.cpu_auto_value) ||
        !config_byte_value_valid(cfg->fan_modes.cpu_manual_value) ||
        !config_byte_value_valid(cfg->fan_modes.cpu_turbo_value) ||
        !config_byte_value_valid(cfg->fan_modes.gpu_auto_value) ||
        !config_byte_value_valid(cfg->fan_modes.gpu_manual_value) ||
        !config_byte_value_valid(cfg->fan_modes.gpu_turbo_value))
        return -1;

    return 0;
}
