#include "util/number.h"

int clamp_int(const int value, const int min, const int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
