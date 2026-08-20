#include "config/parse.h"

#include "util/json.h"

#include <stdio.h>

int config_invalid(const char *message)
{
    fprintf(stderr, "invalid config: %s\n", message);
    return -1;
}

bool config_optional_int_key(const char *json, const char *key, int *value)
{
    if (!json_find_key(json, key))
        return true;

    return json_int_key_checked(json, key, value);
}

bool config_optional_bool_key(const char *json, const char *key, bool *value)
{
    if (!json_find_key(json, key))
        return true;

    return json_bool_key_checked(json, key, value);
}

bool config_optional_string_key(const char *json, const char *key,
                                char *out, const size_t out_len)
{
    if (!json_find_key(json, key))
        return true;

    return json_string_key_checked(json, key, out, out_len);
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
    return value >= 0 && value <= 255;
}

bool config_percent_value_valid(const int value)
{
    return value >= 1 && value <= 100;
}

bool config_speed_value_valid(const int value)
{
    return value >= 0 && value <= 100;
}
