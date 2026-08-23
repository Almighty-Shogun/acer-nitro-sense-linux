#include "config/parse.h"

#include "util/json.h"
#include "util/number.h"

#include <stdio.h>

/**
 * Return whether a JSON object contains a key.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
static bool config_key_present(const char* json, const char* key)
{
    return json_find_key(json, key) != NULL;
}

/**
 * Return whether an integer is inside an allowed range.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
static bool config_int_value_valid(const int value, const int min, const int max)
{
    return value >= min && value <= max;
}

/**
 * Report an invalid model configuration value.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
int config_invalid(const char* message)
{
    fprintf(stderr, "invalid config: %s\n", message);

    return -1;
}

/**
 * Read an optional integer config key.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_optional_int_key(const char* json, const char* key, int* value)
{
    if (!config_key_present(json, key))
        return true;

    return json_int_key_checked(json, key, value);
}

/**
 * Read an optional boolean config key.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_optional_bool_key(const char* json, const char* key, bool* value)
{
    if (!config_key_present(json, key))
        return true;

    return json_bool_key_checked(json, key, value);
}

/**
 * Read an optional string config key.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_optional_string_key(const char* json, const char* key, char* out, const size_t out_len)
{
    if (!config_key_present(json, key))
        return true;

    return json_string_key_checked(json, key, out, out_len);
}

/**
 * Read an optional clamped integer config key.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_optional_clamped_int_key(const char* json, const char* key, int* value, const int min, const int max)
{
    if (!config_optional_int_key(json, key, value))
        return false;

    *value = clamp_int(*value, min, max);

    return true;
}

/**
 * Read a required integer config key.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_required_int_key(const char* json, const char* key, int* value)
{
    return json_int_key_checked(json, key, value);
}

/**
 * Read a required string config key.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_required_string_key(const char* json, const char* key, char* out, const size_t out_len)
{
    return json_string_key_checked(json, key, out, out_len) && out[0] != '\0';
}

/**
 * Return whether a value fits in an EC byte.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_byte_value_valid(const int value)
{
    return config_int_value_valid(value, 0, 255);
}

/**
 * Return whether a value is a fan percentage.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_percent_value_valid(const int value)
{
    return config_int_value_valid(value, 1, 100);
}

/**
 * Return whether a value is a configured fan speed.
 *
 * Model JSON is treated as the hardware contract for a machine. Validation
 * stays strict here so bad EC registers or incomplete presets fail before the
 * daemon starts.
 */
bool config_speed_value_valid(const int value)
{
    return config_int_value_valid(value, 0, 100);
}
