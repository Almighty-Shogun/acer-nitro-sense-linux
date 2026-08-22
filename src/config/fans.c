#include "config/sections.h"

#include "util/json.h"
#include "config/parse.h"

#include <stdio.h>
#include <string.h>

/**
 * Parse one fan curve threshold.
 *
 * Curve entries must define ramp-up and ramp-down temperatures plus the fan
 * speed that should be applied while the entry is active.
 */
static bool parse_curve_entry(const char* threshold_json, struct threshold* threshold)
{
    const bool has_up = config_required_int_key(threshold_json, "up", &threshold->up);
    const bool has_down = config_required_int_key(threshold_json, "down", &threshold->down);
    const bool has_speed = config_required_int_key(threshold_json, "speed", &threshold->speed);

    if (!has_up || !has_down || !has_speed)
        return false;

    return threshold->up >= 0
        && threshold->up <= 130
        && threshold->down >= 0
        && threshold->down <= 130
        && threshold->down <= threshold->up
        && config_speed_value_valid(threshold->speed);
}

/**
 * Parse the automatic fan curve.
 *
 * The curve is required for every fan because daemon-auto mode depends on at
 * least one validated threshold to decide safe speed changes.
 */
static int parse_curve(const char* fan_json, struct fan_config* fan)
{
    const char* end;
    const char* p = json_find_array(fan_json, "curve", &end);

    if (!p)
        return config_invalid("fan curve is required");

    while (p < end && fan->curve_len < ANS_MAX_THRESHOLDS)
    {
        const char* obj_end;
        const char* obj = json_next_object(p, end, &obj_end);

        char buf[256];
        struct threshold* t = &fan->curve[fan->curve_len];

        if (!obj) break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));

        if (!parse_curve_entry(buf, t))
            return config_invalid("fan curve entries require up/down temperatures and 0-100 speed");

        fan->curve_len++;
        p = obj_end;
    }

    return fan->curve_len > 0 ? 0 : config_invalid("fan curve must contain at least one point");
}

/**
 * Copy the fan sensor group into the control sensor group.
 *
 * Fans default to controlling from the same sensor group they display. Model
 * profiles can override this when one component should follow another sensor.
 */
static void copy_default_control_sensor_group(struct fan_config* fan)
{
    for (size_t i = 0; i + 1 < sizeof(fan->control_sensor_group); i++)
    {
        fan->control_sensor_group[i] = fan->sensor_group[i];

        if (fan->sensor_group[i] == '\0') break;
    }

    fan->control_sensor_group[sizeof(fan->control_sensor_group) - 1] = '\0';
}

/**
 * Parse required fan identity fields.
 *
 * The id is used by commands, the name is shown in status output, and the
 * sensor group chooses which hwmon sensors can feed temperature data.
 */
static bool parse_fan_identity(const char* fan_json, struct fan_config* fan)
{
    const bool has_id = config_required_string_key(fan_json, "id", fan->id, sizeof(fan->id));
    const bool has_name = config_required_string_key(fan_json, "name", fan->name, sizeof(fan->name));

    const bool has_sensor_group = config_required_string_key(
        fan_json,
        "sensor_group",
        fan->sensor_group,
        sizeof(fan->sensor_group)
    );

    return has_id && has_name && has_sensor_group;
}

/**
 * Parse fan sensor behavior.
 *
 * Sensor options decide which group controls the fan and whether matching
 * sensor devices should be kept awake for live temperature readings.
 */
static int parse_fan_sensor_config(const char* fan_json, struct fan_config* fan)
{
    copy_default_control_sensor_group(fan);

    const bool control_sensor_group_valid = config_optional_string_key(
        fan_json,
        "control_sensor_group",
        fan->control_sensor_group,
        sizeof(fan->control_sensor_group)
    );

    if (!control_sensor_group_valid)
        return config_invalid("fan control_sensor_group must be a plain string");

    fan->keep_awake = false;

    if (!config_optional_bool_key(fan_json, "keep_awake", &fan->keep_awake))
        return config_invalid("fan keep_awake must be true or false");

    fan->sensor_power_control[0] = '\0';

    if (fan->keep_awake)
        snprintf(fan->sensor_power_control, sizeof(fan->sensor_power_control), "on");

    const bool sensor_power_control_valid = config_optional_string_key(
        fan_json,
        "sensor_power_control",
        fan->sensor_power_control,
        sizeof(fan->sensor_power_control)
    );

    if (!sensor_power_control_valid)
        return config_invalid("fan sensor_power_control must be a plain string");

    if (strcmp(fan->sensor_power_control, "on") != 0 && strcmp(fan->sensor_power_control, "auto") != 0)
        fan->sensor_power_control[0] = '\0';

    return 0;
}

/**
 * Parse required fan EC registers.
 *
 * Fan RPM reads and speed writes are byte-addressed EC registers and must be
 * valid before the daemon can safely touch hardware.
 */
static int parse_fan_ec_registers(const char* fan_json, struct fan_config* fan)
{
    const bool has_read_register = config_required_int_key(fan_json, "read_register", &fan->read_register);
    const bool has_write_register = config_required_int_key(fan_json, "write_register", &fan->write_register);

    if (!has_read_register || !has_write_register)
        return config_invalid("fan entries require byte read_register and write_register");

    if (!config_byte_value_valid(fan->read_register) || !config_byte_value_valid(fan->write_register))
        return config_invalid("fan entries require byte read_register and write_register");

    return 0;
}

/**
 * Parse optional temperature EC registers.
 *
 * Models can omit temperature registers when Linux sensors should be used.
 * Present registers still need byte validation because they are read from EC.
 */
static int parse_fan_temperature_registers(const char* fan_json, struct fan_config* fan)
{
    fan->temperature_register = -1;
    fan->control_temperature_register = -1;

    const bool temperature_register_valid = config_optional_int_key(
        fan_json,
        "temperature_register",
        &fan->temperature_register
    );

    const bool control_temperature_register_valid = config_optional_int_key(
        fan_json,
        "control_temperature_register",
        &fan->control_temperature_register
    );

    if (!temperature_register_valid || !control_temperature_register_valid)
        return config_invalid("fan temperature registers are invalid");

    if (fan->temperature_register != -1 && !config_byte_value_valid(fan->temperature_register))
        return config_invalid("fan temperature_register must be a byte register");

    if (fan->control_temperature_register != -1 && !config_byte_value_valid(fan->control_temperature_register))
        return config_invalid("fan control_temperature_register must be a byte register");

    return 0;
}

/**
 * Fill fan numeric defaults.
 *
 * Defaults keep older compact model profiles valid while allowing newer
 * profiles to tune RPM ranges, write ranges, and fallback fan speed.
 */
static void set_fan_numeric_defaults(struct fan_config* fan)
{
    fan->read_min = 0;
    fan->read_max = 6000;
    fan->write_min = 0;
    fan->write_max = 100;
    fan->reset_speed = 50;
    fan->missing_temperature_speed_percent = 0;
}

/**
 * Parse optional fan numeric limits.
 *
 * Numeric limits are split from range validation so parse failures and
 * semantically invalid ranges can return precise model errors.
 */
static int parse_fan_numeric_limits(const char* fan_json, struct fan_config* fan)
{
    set_fan_numeric_defaults(fan);

    const bool read_min_valid = config_optional_int_key(fan_json, "read_min", &fan->read_min);
    const bool read_max_valid = config_optional_int_key(fan_json, "read_max", &fan->read_max);
    const bool write_min_valid = config_optional_int_key(fan_json, "write_min", &fan->write_min);
    const bool write_max_valid = config_optional_int_key(fan_json, "write_max", &fan->write_max);
    const bool reset_speed_valid = config_optional_int_key(fan_json, "reset_speed", &fan->reset_speed);

    const bool missing_temperature_speed_valid = config_optional_clamped_int_key(
        fan_json,
        "missing_temperature_speed_percent",
        &fan->missing_temperature_speed_percent,
        0,
        100
    );

    const bool numeric_limits_valid = read_min_valid
        && read_max_valid
        && write_min_valid
        && write_max_valid
        && reset_speed_valid
        && missing_temperature_speed_valid;

    if (!numeric_limits_valid)
        return config_invalid("fan numeric limits are invalid");

    return 0;
}

/**
 * Validate parsed fan numeric ranges.
 *
 * Read ranges must be ascending RPM ranges, write ranges must fit 0-100, and
 * the reset speed must be a writable value.
 */
static int validate_fan_numeric_ranges(const struct fan_config* fan)
{
    const bool read_range_valid = fan->read_min >= 0 && fan->read_max > fan->read_min;

    const bool write_range_valid = fan->write_min >= 0
        && fan->write_max <= 100
        && fan->write_max > fan->write_min;

    const bool reset_speed_valid = fan->reset_speed >= fan->write_min && fan->reset_speed <= fan->write_max;

    return read_range_valid
           && write_range_valid
           && reset_speed_valid
               ? 0
               : config_invalid("fan numeric ranges are invalid");
}

/**
 * Parse one fan definition.
 *
 * A fan definition is valid only when identity, sensor behavior, EC registers,
 * numeric limits, and the automatic curve are all accepted.
 */
static int parse_fan(const char* fan_json, struct fan_config* fan)
{
    if (!parse_fan_identity(fan_json, fan))
        return config_invalid("fan entries require non-empty id, name, and sensor_group");

    if (parse_fan_sensor_config(fan_json, fan) < 0)
        return -1;

    if (parse_fan_ec_registers(fan_json, fan) < 0)
        return -1;

    if (parse_fan_temperature_registers(fan_json, fan) < 0)
        return -1;

    if (parse_fan_numeric_limits(fan_json, fan) < 0)
        return -1;

    if (validate_fan_numeric_ranges(fan) < 0)
        return -1;

    return parse_curve(fan_json, fan);
}

/**
 * Parse fan definitions.
 *
 * The fans array is required and must contain at least one fully validated fan
 * entry before model loading can continue.
 */
int config_parse_fans(const char* json, struct ans_config* cfg)
{
    const char* end;
    const char* p = json_find_array(json, "fans", &end);

    if (!p)
        return -1;

    while (p < end && cfg->fan_len < ANS_MAX_FANS)
    {
        const char* obj_end;
        const char* obj = json_next_object(p, end, &obj_end);

        char buf[8192];
        struct fan_config* fan = &cfg->fans[cfg->fan_len];

        if (!obj) break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));

        if (parse_fan(buf, fan) < 0)
            return -1;

        cfg->fan_len++;
        p = obj_end;
    }

    return cfg->fan_len > 0 ? 0 : -1;
}
