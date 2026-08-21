#include "util/number.h"

#include <errno.h>
#include <stdlib.h>

int clamp_int(const int value, const int min, const int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

bool parse_int_range(const char *text, const int min_value,
                     const int max_value, const int base, int *value)
{
    char *end;

    errno = 0;
    const long parsed = strtol(text, &end, base);
    if (errno != 0 || end == text || *end != '\0' ||
        parsed < min_value || parsed > max_value)
        return false;

    *value = (int)parsed;
    return true;
}
