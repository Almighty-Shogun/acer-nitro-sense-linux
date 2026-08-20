#ifndef ANS_UTIL_STRING_H
#define ANS_UTIL_STRING_H

#include <stdbool.h>
#include <stddef.h>

bool string_contains_case(const char *haystack, const char *needle);
bool string_copy(char *out, size_t out_len, const char *text);
void trim_ascii(char *s);

#endif
