#ifndef ANS_UTIL_STRING_H
#define ANS_UTIL_STRING_H

#include <stdbool.h>

bool string_contains_case(const char *haystack, const char *needle);
void trim_ascii(char *s);

#endif
