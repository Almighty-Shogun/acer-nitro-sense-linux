#include "client/parse.h"

#include "util/number.h"

/**
 * Parse percent.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
bool client_parse_percent(const char* text, int* percent)
{
    return parse_int_range(text, 1, 100, 10, percent);
}

/**
 * Parse byte value.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
bool client_parse_byte_value(const char* text, int* value)
{
    return parse_int_range(text, 0, 255, 0, value);
}
