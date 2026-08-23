#include "util/number.h"

#include <errno.h>
#include <stdlib.h>

/**
 * Clamp an integer into an inclusive range.
 *
 * Fan percentages and EC byte values both use inclusive bounds, so this helper
 * keeps range enforcement explicit at the call site.
 */
int clamp_int(const int value, const int min, const int max)
{
    if (value < min)
        return min;

    if (value > max)
        return max;

    return value;
}

/**
 * Parse an integer and validate it against an inclusive range.
 *
 * Command parsers use this for decimal and hexadecimal values without accepting
 * trailing garbage or silently overflowing the target integer type.
 */
bool parse_int_range(const char* text, const int min_value, const int max_value, const int base, int* value)
{
    char* end;

    errno = 0;
    const long parsed = strtol(text, &end, base);

    if (errno != 0 || end == text || *end != '\0' || parsed < min_value || parsed > max_value)
        return false;

    *value = (int)parsed;

    return true;
}
