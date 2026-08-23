#ifndef ANS_UTIL_NUMBER_H
#define ANS_UTIL_NUMBER_H

#include <stdbool.h>

/**
 * Clamp an integer to an inclusive range.
 *
 * Fan and config code use this when safety policy produces bounded values.
 */
int clamp_int(int value, int min, int max);

/**
 * Parse a complete integer token inside an inclusive range.
 *
 * Partial conversions and out-of-range values are rejected instead of silently
 * clamped.
 */
bool parse_int_range(const char* text, int min_value, int max_value, int base, int* value);

#endif
