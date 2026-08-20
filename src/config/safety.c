#include "config/sections.h"

#include "config/parse.h"
#include "util/json.h"

int config_parse_safety(const char *json, struct ans_config *cfg)
{
    const char *safety = json_find_key(json, "safety");

    cfg->safety.min_speed_percent = 30;
    cfg->safety.min_speed_temperature_c = 60;
    cfg->safety.critical_speed_percent = 100;
    cfg->safety.critical_full_speed = true;
    cfg->safety.critical_step_percent = 20;
    cfg->safety.critical_consecutive_samples = 1;
    cfg->safety.critical_release_temperature_c = cfg->critical_temperature_c - 5;
    cfg->safety.auto_ramp_up_percent = 0;
    cfg->safety.auto_ramp_bypass_temperature_c = 0;
    cfg->safety.missing_temperature_speed_percent = 60;
    cfg->safety.max_ec_read_failures = 3;
    cfg->safety.max_ec_write_failures = 3;

    if (!safety)
        return 0;

    if (!config_optional_clamped_int_key(safety, "min_speed_percent",
                                         &cfg->safety.min_speed_percent, 1,
                                         100) ||
        !config_optional_clamped_int_key(safety, "min_speed_temperature_c",
                                         &cfg->safety.min_speed_temperature_c,
                                         1, 130) ||
        !config_optional_clamped_int_key(safety, "critical_speed_percent",
                                         &cfg->safety.critical_speed_percent,
                                         1, 100) ||
        !config_optional_bool_key(safety, "critical_full_speed",
                                  &cfg->safety.critical_full_speed) ||
        !config_optional_clamped_int_key(safety, "critical_step_percent",
                                         &cfg->safety.critical_step_percent,
                                         1, 100) ||
        !config_optional_clamped_int_key(
            safety, "critical_consecutive_samples",
            &cfg->safety.critical_consecutive_samples, 1, 10) ||
        !config_optional_clamped_int_key(
            safety, "critical_release_temperature_c",
            &cfg->safety.critical_release_temperature_c, 1,
            cfg->critical_temperature_c) ||
        !config_optional_clamped_int_key(safety, "auto_ramp_up_percent",
                                         &cfg->safety.auto_ramp_up_percent, 0,
                                         100) ||
        !config_optional_clamped_int_key(
            safety, "auto_ramp_bypass_temperature_c",
            &cfg->safety.auto_ramp_bypass_temperature_c, 0,
            cfg->critical_temperature_c) ||
        !config_optional_clamped_int_key(
            safety, "missing_temperature_speed_percent",
            &cfg->safety.missing_temperature_speed_percent, 1, 100) ||
        !config_optional_clamped_int_key(safety, "max_ec_read_failures",
                                         &cfg->safety.max_ec_read_failures, 1,
                                         100) ||
        !config_optional_clamped_int_key(safety, "max_ec_write_failures",
                                         &cfg->safety.max_ec_write_failures, 1,
                                         100))
        return -1;

    return 0;
}
