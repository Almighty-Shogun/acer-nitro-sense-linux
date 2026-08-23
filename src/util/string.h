#ifndef ANS_UTIL_STRING_H
#define ANS_UTIL_STRING_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Return whether text contains a substring case-insensitively.
 *
 * Hardware and sensor matching use this to tolerate Linux naming variations.
 */
bool string_contains_case(const char* haystack, const char* needle);

/**
 * Copy a string into a bounded destination.
 *
 * The function returns false when truncation would be required.
 */
bool string_copy(char* out, size_t out_len, const char* text);

/**
 * Copy a fixed-length string span into a bounded destination.
 *
 * This is used by parsers that already know token boundaries.
 */
bool string_copy_span(char* out, size_t out_len, const char* start, size_t len);

/**
 * Trim leading and trailing ASCII whitespace in place.
 *
 * The helper is intentionally ASCII-only because config and command tokens use
 * ASCII syntax.
 */
void trim_ascii(char* s);

#endif
