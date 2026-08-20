#ifndef ANS_CONFIG_PARSE_H
#define ANS_CONFIG_PARSE_H

#include <stdbool.h>
#include <stddef.h>

int config_invalid(const char *message);
bool config_optional_int_key(const char *json, const char *key, int *value);
bool config_optional_bool_key(const char *json, const char *key, bool *value);
bool config_optional_string_key(const char *json, const char *key,
                                char *out, size_t out_len);
bool config_required_int_key(const char *json, const char *key, int *value);
bool config_required_string_key(const char *json, const char *key,
                                char *out, size_t out_len);
bool config_byte_value_valid(int value);
bool config_percent_value_valid(int value);
bool config_speed_value_valid(int value);

#endif
