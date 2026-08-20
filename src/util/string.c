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
