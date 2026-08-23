#include "util/string.h"

#include <ctype.h>
#include <string.h>

/**
 * Return whether text contains a substring case-insensitively.
 *
 * Linux exposes device names with driver-specific casing, so hardware matching
 * uses this helper instead of duplicating case-folding loops.
 */
bool string_contains_case(const char* haystack, const char* needle)
{
    const size_t len = strlen(needle);

    if (len == 0)
        return true;

    for (const char* h = haystack; *h; h++)
    {
        size_t i = 0;

        while (i < len && h[i] && tolower((unsigned char)h[i]) == tolower((unsigned char)needle[i]))
            i++;

        if (i == len)
            return true;
    }

    return false;
}

/**
 * Copy a string into a fixed-size destination.
 *
 * The destination is always NUL-terminated when it has space, and the return
 * value tells callers whether the original text fit without truncation.
 */
bool string_copy(char* out, const size_t out_len, const char* text)
{
    const size_t len = text ? strlen(text) : 0;

    if (out_len == 0)
        return false;

    if (!text)
    {
        out[0] = '\0';

        return true;
    }

    if (len >= out_len)
    {
        memcpy(out, text, out_len - 1);

        out[out_len - 1] = '\0';

        return false;
    }

    memcpy(out, text, len + 1);

    return true;
}

/**
 * Copy a bounded string span into a fixed-size destination.
 *
 * Parser helpers often know token boundaries without having a temporary
 * NUL-terminated string. This copies that span directly and reports truncation.
 */
bool string_copy_span(char* out, const size_t out_len, const char* start, size_t len)
{
    if (out_len == 0)
        return false;

    if (!start)
    {
        out[0] = '\0';

        return true;
    }

    if (len >= out_len)
    {
        len = out_len - 1;

        memcpy(out, start, len);

        out[len] = '\0';

        return false;
    }

    memcpy(out, start, len);

    out[len] = '\0';

    return true;
}

/**
 * Trim leading and trailing ASCII whitespace in place.
 *
 * Configuration and command tokens use ASCII syntax, so locale-sensitive
 * whitespace handling would only make parsing less predictable.
 */
void trim_ascii(char* s)
{
    const char* start = s;

    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')
        start++;

    if (start != s)
        memmove(s, start, strlen(start) + 1);

    char* end = s + strlen(s);

    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r'))
        *--end = '\0';
}
