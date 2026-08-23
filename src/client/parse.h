#ifndef ANS_CLIENT_PARSE_H
#define ANS_CLIENT_PARSE_H

#include <stdbool.h>

/**
 * Parse a user-supplied EC byte value.
 *
 * Debug commands accept decimal or prefixed numeric input but always clamp to
 * one EC register byte.
 */
bool client_parse_byte_value(const char* text, int* value);

/**
 * Parse a user-supplied fan percentage.
 *
 * Manual fan commands accept only complete numeric values in the 1-100 range.
 */
bool client_parse_percent(const char* text, int* percent);

#endif
