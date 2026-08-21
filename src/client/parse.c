#include "client/parse.h"

#include "util/number.h"

bool client_parse_percent(const char *text, int *percent)
{
    return parse_int_range(text, 1, 100, 10, percent);
}

bool client_parse_byte_value(const char *text, int *value)
{
    return parse_int_range(text, 0, 255, 0, value);
}
