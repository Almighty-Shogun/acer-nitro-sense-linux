#include "commands/parser/parser.h"
#include "commands/parser/internal.h"

#include "util/number.h"

#include <string.h>

/**
 * Skip command whitespace.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
const char* command_skip_space(const char* p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;

    return p;
}

/**
 * Read one command token.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_read_token(const char** cursor, char* out, const size_t out_len)
{
    const char* p = command_skip_space(*cursor);

    size_t len = 0;

    if (*p == '\0')
        return false;

    while (p[len] && p[len] != ' ' && p[len] != '\t' && p[len] != '\r' && p[len] != '\n')
        len++;

    if (len == 0 || len >= out_len)
        return false;

    memcpy(out, p, len);

    out[len] = '\0';
    *cursor = p + len;

    return true;
}

/**
 * Return whether a token is in an allowed string set.
 *
 * Command handlers use this to keep action validation table-driven instead of
 * repeating chains of string comparisons.
 */
bool command_token_in(const char* token, const char* const* values, const size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (strcmp(token, values[i]) == 0)
            return true;
    }

    return false;
}

/**
 * Return whether a value is in an allowed integer set.
 *
 * Numeric command arguments such as keyboard brightness steps can then be
 * checked against the exact model-supported values.
 */
bool command_int_in(const int value, const int* values, const size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (value == values[i])
            return true;
    }

    return false;
}

/**
 * Read the first token from a command line.
 *
 * Dispatchers use this once to select a command table entry before handing the
 * full command line to the owning feature handler.
 */
bool command_first_token(const char* cmd, char* token, const size_t token_len)
{
    const char* cursor = cmd;

    return command_read_token(&cursor, token, token_len);
}

/**
 * Read the second token from a command line.
 *
 * Feature dispatchers use this to select a subcommand table entry after the
 * top-level daemon registry has selected the owning command.
 */
bool command_second_token(const char* cmd, char* token, const size_t token_len)
{
    char first[32];
    const char* cursor = cmd;

    return command_read_token(&cursor, first, sizeof(first)) && command_read_token(&cursor, token, token_len);
}

/**
 * Parse a one-action command and validate the action against an allowed set.
 *
 * Feature handlers use this when a command has a closed action list, such as
 * `fan-mode auto|manual|turbo`.
 */
bool command_parse_enum_action(
    const char* cmd,
    const char* expected_command,
    const char* const* values,
    const size_t values_len,
    char* action,
    const size_t action_len
)
{
    return command_parse_action(cmd, expected_command, action, action_len) && command_token_in(action, values, values_len);
}

/**
 * Return whether only trailing space.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_has_only_trailing_space(const char* p)
{
    return *command_skip_space(p) == '\0';
}

/**
 * Parse percent token.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_parse_percent_token(const char* text, int* percent)
{
    return parse_int_range(text, 1, 100, 10, percent);
}

/**
 * Parse int token.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_parse_int_token(const char* text, const int min_value, const int max_value, int* value)
{
    return parse_int_range(text, min_value, max_value, 0, value);
}

/**
 * Parse a command with one action token.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_parse_action(const char* cmd, const char* expected_command, char* action, const size_t action_len)
{
    char command[32];

    const char* cursor = cmd;

    if (!command_read_token(&cursor, command, sizeof(command)) || strcmp(command, expected_command) != 0)
        return false;

    if (!command_read_token(&cursor, action, action_len))
        return false;

    return command_has_only_trailing_space(cursor);
}

/**
 * Parse a command with an action and sub-action.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_parse_sub_action(
    const char* cmd,
    const char* expected_command,
    const char* expected_action,
    char* value,
    const size_t value_len
)
{
    char action[16], command[32];

    const char* cursor = cmd;

    if (!command_read_token(&cursor, command, sizeof(command)) || strcmp(command, expected_command) != 0)
        return false;

    if (!command_read_token(&cursor, action, sizeof(action)) || strcmp(action, expected_action) != 0)
        return false;

    if (!command_read_token(&cursor, value, value_len))
        return false;

    return command_has_only_trailing_space(cursor);
}

/**
 * Parse an integer command action.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_parse_int_action(
    const char* cmd,
    const char* expected_command,
    const int min_value,
    const int max_value,
    int* value
)
{
    char value_text[16];

    return command_parse_action(cmd, expected_command, value_text, sizeof(value_text))
        && command_parse_int_token(value_text, min_value, max_value, value);
}

/**
 * Parse an integer command sub-action.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_parse_int_sub_action(
    const char* cmd,
    const char* expected_command,
    const char* expected_action,
    const int min_value,
    const int max_value,
    int* value
)
{
    char value_text[16];

    return command_parse_sub_action(cmd, expected_command, expected_action, value_text, sizeof(value_text))
        && command_parse_int_token(value_text, min_value, max_value, value);
}
