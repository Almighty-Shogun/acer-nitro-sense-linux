#ifndef ANS_UTIL_NUMBER_H
#define ANS_UTIL_NUMBER_H

#include <stdbool.h>

int clamp_int(int value, int min, int max);
bool parse_int_range(const char *text, int min_value, int max_value,
                     int base, int *value);

#endif
