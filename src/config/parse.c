#include "config/parse.h"

#include "util/json.h"
#include "util/number.h"

#include <stdio.h>

static bool config_key_present(const char *json, const char *key)
{
    return json_find_key(json, key) != NULL;
}

static bool config_int_value_valid(const int value, const int min,
                                   const int max)
{
    return value >= min && value <= max;
}

int config_invalid(const char *message)
{
    fprintf(stderr, "invalid config: %s\n", message);
    return -1;
}

bool config_optional_int_key(const char *json, const char *key, int *value)
{
    if (!config_key_present(json, key))
        return true;

    return json_int_key_checked(json, key, value);
}

bool config_optional_bool_key(const char *json, const char *key, bool *value)
{
    if (!config_key_present(json, key))
        return true;

    return json_bool_key_checked(json, key, value);
}

bool config_optional_string_key(const char *json, const char *key,
                                char *out, const size_t out_len)
{
    if (!config_key_present(json, key))
        return true;

    return json_string_key_checked(json, key, out, out_len);
}

bool config_optional_clamped_int_key(const char *json, const char *key,
                                     int *value, const int min, const int max)
{
    if (!config_optional_int_key(json, key, value))
        return false;

    *value = clamp_int(*value, min, max);
    return true;
}

bool config_required_int_key(const char *json, const char *key, int *value)
{
    return json_int_key_checked(json, key, value);
}

bool config_required_string_key(const char *json, const char *key,
                                char *out, const size_t out_len)
{
    return json_string_key_checked(json, key, out, out_len) && out[0] != '\0';
}

bool config_byte_value_valid(const int value)
{
    return config_int_value_valid(value, 0, 255);
}

bool config_percent_value_valid(const int value)
{
    return config_int_value_valid(value, 1, 100);
}

bool config_speed_value_valid(const int value)
{
    return config_int_value_valid(value, 0, 100);
}
