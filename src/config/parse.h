#ifndef ANS_CONFIG_PARSE_H
#define ANS_CONFIG_PARSE_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Report an invalid model configuration value.
 *
 * Config parsing returns a uniform failure code while this helper prints the
 * reason that should be fixed in the model JSON.
 */
int config_invalid(const char* message);

/**
 * Read an optional integer config key.
 *
 * Missing keys leave the caller's existing default in place.
 */
bool config_optional_int_key(const char* json, const char* key, int* value);

/**
 * Read an optional boolean config key.
 *
 * Missing keys leave the caller's existing default in place.
 */
bool config_optional_bool_key(const char* json, const char* key, bool* value);

/**
 * Read an optional string config key.
 *
 * Missing keys leave the caller's existing default in place and successful
 * reads are bounded by the destination buffer.
 */
bool config_optional_string_key(const char* json, const char* key, char* out, size_t out_len);

/**
 * Read an optional clamped integer config key.
 *
 * Present values must fit the supplied range; missing values preserve the
 * caller's default.
 */
bool config_optional_clamped_int_key(const char* json, const char* key, int* value, int min, int max);

/**
 * Read a required integer config key.
 *
 * Parsing fails when the key is missing or cannot be converted to an integer.
 */
bool config_required_int_key(const char* json, const char* key, int* value);

/**
 * Read a required string config key.
 *
 * Parsing fails when the key is missing or the destination buffer cannot hold
 * the value.
 */
bool config_required_string_key(const char* json, const char* key, char* out, size_t out_len);

/**
 * Return whether a value fits in an EC byte.
 *
 * EC registers and payloads are byte-sized, so JSON values outside 0-255 are
 * rejected during profile loading.
 */
bool config_byte_value_valid(int value);

/**
 * Return whether a value is a fan percentage.
 *
 * Fan percentages are user-facing values and must stay inside the supported
 * 1-100 control range.
 */
bool config_percent_value_valid(int value);

/**
 * Return whether a value is a configured fan speed.
 *
 * Some configuration fields allow zero for firmware or missing-temperature
 * behavior, unlike direct user fan percentages.
 */
bool config_speed_value_valid(int value);

#endif
