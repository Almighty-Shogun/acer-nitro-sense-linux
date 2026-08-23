#include "config/sections.h"

#include "util/json.h"
#include "config/parse.h"

/**
 * Parse safety policy overrides.
 *
 * Every field is optional because the parser seeds conservative defaults
 * before this helper runs. Invalid values still fail the model load instead of
 * silently weakening runtime protection.
 */
static bool parse_safety_policy(const char* safety, struct ans_config* cfg)
{
    const bool min_speed_valid = config_optional_clamped_int_key(
        safety,
        "min_speed_percent",
        &cfg->safety.min_speed_percent,
        1,
        100
    );

    const bool min_speed_temperature_valid = config_optional_clamped_int_key(
        safety,
        "min_speed_temperature_c",
        &cfg->safety.min_speed_temperature_c,
        1,
        130
    );

    const bool critical_speed_valid = config_optional_clamped_int_key(
        safety,
        "critical_speed_percent",
        &cfg->safety.critical_speed_percent,
        1,
        100
    );

    const bool critical_full_speed_valid = config_optional_bool_key(
        safety,
        "critical_full_speed",
        &cfg->safety.critical_full_speed
    );

    const bool critical_step_valid = config_optional_clamped_int_key(
        safety,
        "critical_step_percent",
        &cfg->safety.critical_step_percent,
        1,
        100
    );

    const bool critical_samples_valid = config_optional_clamped_int_key(
        safety,
        "critical_consecutive_samples",
        &cfg->safety.critical_consecutive_samples,
        1,
        10
    );

    const bool critical_release_temperature_valid = config_optional_clamped_int_key(
        safety,
        "critical_release_temperature_c",
        &cfg->safety.critical_release_temperature_c,
        1,
        cfg->critical_temperature_c
    );

    const bool auto_ramp_up_valid = config_optional_clamped_int_key(
        safety,
        "auto_ramp_up_percent",
        &cfg->safety.auto_ramp_up_percent,
        0,
        100
    );

    const bool auto_ramp_bypass_temperature_valid = config_optional_clamped_int_key(
        safety,
        "auto_ramp_bypass_temperature_c",
        &cfg->safety.auto_ramp_bypass_temperature_c,
        0,
        cfg->critical_temperature_c
    );

    const bool missing_temperature_speed_valid = config_optional_clamped_int_key(
        safety,
        "missing_temperature_speed_percent",
        &cfg->safety.missing_temperature_speed_percent,
        1,
        100
    );

    const bool max_read_failures_valid = config_optional_clamped_int_key(
        safety,
        "max_ec_read_failures",
        &cfg->safety.max_ec_read_failures,
        1,
        100
    );

    const bool max_write_failures_valid = config_optional_clamped_int_key(
        safety,
        "max_ec_write_failures",
        &cfg->safety.max_ec_write_failures,
        1,
        100
    );

    return min_speed_valid
           && min_speed_temperature_valid
           && critical_speed_valid
           && critical_full_speed_valid
           && critical_step_valid
           && critical_samples_valid
           && critical_release_temperature_valid
           && auto_ramp_up_valid
           && auto_ramp_bypass_temperature_valid
           && missing_temperature_speed_valid
           && max_read_failures_valid
           && max_write_failures_valid;
}

/**
 * Parse safety.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
int config_parse_safety(const char* json, struct ans_config* cfg)
{
    const char* safety = json_find_key(json, "safety");

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

    if (!parse_safety_policy(safety, cfg))
        return -1;

    return 0;
}
