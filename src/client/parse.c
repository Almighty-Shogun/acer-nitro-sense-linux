#include "client/parse.h"

#include <errno.h>
#include <stdlib.h>

bool client_parse_percent(const char *text, int *percent)
{
    char *end;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value < 1 || value > 100)
        return false;

    *percent = (int)value;
    return true;
}

bool client_parse_byte_value(const char *text, int *value)
{
    char *end;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || parsed < 0 || parsed > 255)
        return false;

    *value = (int)parsed;
    return true;
}
