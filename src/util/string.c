#include "util/string.h"

#include <ctype.h>
#include <string.h>

bool string_contains_case(const char *haystack, const char *needle)
{
    const size_t nlen = strlen(needle);

    if (nlen == 0)
        return true;

    for (const char *h = haystack; *h; h++) {
        size_t i = 0;

        while (i < nlen && h[i] &&
               tolower((unsigned char)h[i]) == tolower((unsigned char)needle[i]))
            i++;
        if (i == nlen)
            return true;
    }
    return false;
}

bool string_copy(char *out, const size_t out_len, const char *text)
{
    const size_t len = text ? strlen(text) : 0;

    if (out_len == 0)
        return false;

    if (!text) {
        out[0] = '\0';
        return true;
    }

    if (len >= out_len) {
        memcpy(out, text, out_len - 1);
        out[out_len - 1] = '\0';
        return false;
    }

    memcpy(out, text, len + 1);
    return true;
}

bool string_copy_span(char *out, const size_t out_len, const char *start,
                      size_t len)
{
    if (out_len == 0)
        return false;

    if (!start) {
        out[0] = '\0';
        return true;
    }

    if (len >= out_len) {
        len = out_len - 1;
        memcpy(out, start, len);
        out[len] = '\0';
        return false;
    }

    memcpy(out, start, len);
    out[len] = '\0';
    return true;
}

void trim_ascii(char *s)
{
    const char *start = s;

    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')
        start++;

    if (start != s)
        memmove(s, start, strlen(start) + 1);

    char *end = s + strlen(s);

    while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                       end[-1] == '\n' || end[-1] == '\r'))
        *--end = '\0';
}
