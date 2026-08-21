#include "commands/parser/internal.h"

#include "util/number.h"

#include <string.h>

const char *command_skip_space(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    return p;
}

bool command_read_token(const char **cursor, char *out, const size_t out_len)
{
    const char *p = command_skip_space(*cursor);
    size_t len = 0;

    if (*p == '\0')
        return false;

    while (p[len] && p[len] != ' ' && p[len] != '\t' &&
           p[len] != '\r' && p[len] != '\n')
        len++;

    if (len == 0 || len >= out_len)
        return false;

    memcpy(out, p, len);
    out[len] = '\0';
    *cursor = p + len;
    return true;
}

bool command_has_only_trailing_space(const char *p)
{
    return *command_skip_space(p) == '\0';
}

bool command_parse_percent_token(const char *text, int *percent)
{
    return parse_int_range(text, 1, 100, 10, percent);
}

bool command_parse_int_token(const char *text, const int min_value,
                             const int max_value, int *value)
{
    return parse_int_range(text, min_value, max_value, 0, value);
}

bool command_parse_action(const char *cmd, const char *expected_command,
                          char *action, const size_t action_len)
{
    const char *cursor = cmd;
    char command[32];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, expected_command) != 0)
        return false;
    if (!command_read_token(&cursor, action, action_len))
        return false;

    return command_has_only_trailing_space(cursor);
}

bool command_parse_subaction(const char *cmd, const char *expected_command,
                             const char *expected_action, char *value,
                             const size_t value_len)
{
    const char *cursor = cmd;
    char command[32];
    char action[16];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, expected_command) != 0)
        return false;
    if (!command_read_token(&cursor, action, sizeof(action)) ||
        strcmp(action, expected_action) != 0)
        return false;
    if (!command_read_token(&cursor, value, value_len))
        return false;

    return command_has_only_trailing_space(cursor);
}

bool command_parse_int_action(const char *cmd, const char *expected_command,
                              const int min_value, const int max_value,
                              int *value)
{
    char value_text[16];

    return command_parse_action(cmd, expected_command, value_text,
                                sizeof(value_text)) &&
           command_parse_int_token(value_text, min_value, max_value, value);
}

bool command_parse_int_subaction(const char *cmd, const char *expected_command,
                                 const char *expected_action,
                                 const int min_value, const int max_value,
                                 int *value)
{
    char value_text[16];

    return command_parse_subaction(cmd, expected_command, expected_action,
                                   value_text, sizeof(value_text)) &&
           command_parse_int_token(value_text, min_value, max_value, value);
}
