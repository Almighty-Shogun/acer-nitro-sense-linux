#include "config/sections.h"

#include "config/parse.h"
#include "util/json.h"

#include <stdio.h>
#include <string.h>

static int parse_curve(const char *fan_json, struct fan_config *fan)
{
    const char *end;
    const char *p = json_find_array(fan_json, "curve", &end);

    if (!p)
        return config_invalid("fan curve is required");

    while (p < end && fan->curve_len < ANS_MAX_THRESHOLDS) {
        const char *obj_end;
        const char *obj = json_next_object(p, end, &obj_end);
        char buf[256];
        struct threshold *t = &fan->curve[fan->curve_len];

        if (!obj)
            break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));

        if (!config_required_int_key(buf, "up", &t->up) ||
            !config_required_int_key(buf, "down", &t->down) ||
            !config_required_int_key(buf, "speed", &t->speed) ||
            t->up < 0 || t->up > 130 || t->down < 0 || t->down > 130 ||
            t->down > t->up || !config_speed_value_valid(t->speed))
            return config_invalid(
                "fan curve entries require up/down temperatures and 0-100 speed");

        fan->curve_len++;
        p = obj_end;
    }

    return fan->curve_len > 0 ? 0 :
        config_invalid("fan curve must contain at least one point");
}

static void copy_default_control_sensor_group(struct fan_config *fan)
{
    for (size_t i = 0; i + 1 < sizeof(fan->control_sensor_group); i++) {
        fan->control_sensor_group[i] = fan->sensor_group[i];
        if (fan->sensor_group[i] == '\0')
            break;
    }
    fan->control_sensor_group[sizeof(fan->control_sensor_group) - 1] = '\0';
}

int config_parse_fans(const char *json, struct ans_config *cfg)
{
    const char *end;
    const char *p = json_find_array(json, "fans", &end);

    if (!p)
        return -1;

    while (p < end && cfg->fan_len < ANS_MAX_FANS) {
        const char *obj_end;
        const char *obj = json_next_object(p, end, &obj_end);
        char buf[8192];
        struct fan_config *fan = &cfg->fans[cfg->fan_len];

        if (!obj)
            break;

        json_copy_slice(obj, obj_end, buf, sizeof(buf));
        if (!config_required_string_key(buf, "id", fan->id, sizeof(fan->id)) ||
            !config_required_string_key(buf, "name", fan->name,
                                        sizeof(fan->name)) ||
            !config_required_string_key(buf, "sensor_group", fan->sensor_group,
                                        sizeof(fan->sensor_group)))
            return config_invalid(
                "fan entries require non-empty id, name, and sensor_group");

        copy_default_control_sensor_group(fan);
        if (!config_optional_string_key(buf, "control_sensor_group",
                                        fan->control_sensor_group,
                                        sizeof(fan->control_sensor_group)))
            return config_invalid(
                "fan control_sensor_group must be a plain string");

        fan->keep_awake = false;
        if (!config_optional_bool_key(buf, "keep_awake", &fan->keep_awake))
            return config_invalid("fan keep_awake must be true or false");

        fan->sensor_power_control[0] = '\0';
        if (fan->keep_awake)
            snprintf(fan->sensor_power_control,
                     sizeof(fan->sensor_power_control), "on");
        if (!config_optional_string_key(buf, "sensor_power_control",
                                        fan->sensor_power_control,
                                        sizeof(fan->sensor_power_control)))
            return config_invalid(
                "fan sensor_power_control must be a plain string");
        if (strcmp(fan->sensor_power_control, "on") != 0 &&
            strcmp(fan->sensor_power_control, "auto") != 0)
            fan->sensor_power_control[0] = '\0';

        if (!config_required_int_key(buf, "read_register",
                                     &fan->read_register) ||
            !config_required_int_key(buf, "write_register",
                                     &fan->write_register) ||
            !config_byte_value_valid(fan->read_register) ||
            !config_byte_value_valid(fan->write_register))
            return config_invalid(
                "fan entries require byte read_register and write_register");

        fan->temperature_register = -1;
        fan->control_temperature_register = -1;
        if (!config_optional_int_key(buf, "temperature_register",
                                     &fan->temperature_register) ||
            !config_optional_int_key(buf, "control_temperature_register",
                                     &fan->control_temperature_register))
            return config_invalid("fan temperature registers are invalid");
        if (fan->temperature_register != -1 &&
            !config_byte_value_valid(fan->temperature_register))
            return config_invalid("fan temperature_register must be a byte register");
        if (fan->control_temperature_register != -1 &&
            !config_byte_value_valid(fan->control_temperature_register))
            return config_invalid(
                "fan control_temperature_register must be a byte register");

        fan->read_min = 0;
        fan->read_max = 6000;
        fan->write_min = 0;
        fan->write_max = 100;
        fan->reset_speed = 50;
        fan->missing_temperature_speed_percent = 0;
        if (!config_optional_int_key(buf, "read_min", &fan->read_min) ||
            !config_optional_int_key(buf, "read_max", &fan->read_max) ||
            !config_optional_int_key(buf, "write_min", &fan->write_min) ||
            !config_optional_int_key(buf, "write_max", &fan->write_max) ||
            !config_optional_int_key(buf, "reset_speed", &fan->reset_speed) ||
            !config_optional_int_key(buf, "missing_temperature_speed_percent",
                                     &fan->missing_temperature_speed_percent))
            return config_invalid("fan numeric limits are invalid");

        if (fan->read_min < 0 || fan->read_max <= fan->read_min ||
            fan->write_min < 0 || fan->write_max > 100 ||
            fan->write_max <= fan->write_min ||
            fan->reset_speed < fan->write_min ||
            fan->reset_speed > fan->write_max)
            return config_invalid("fan numeric ranges are invalid");

        fan->missing_temperature_speed_percent =
            clamp_int(fan->missing_temperature_speed_percent, 0, 100);

        if (parse_curve(buf, fan) < 0)
            return -1;

        cfg->fan_len++;
        p = obj_end;
    }

    return cfg->fan_len > 0 ? 0 : -1;
}
