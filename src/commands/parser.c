#include "commands/parser.h"
#include "commands/parser_internal.h"

#include <string.h>

bool parse_set_command(const char *cmd, char *fan, size_t fan_len, int *percent)
{
    const char *cursor = cmd;
    char command[16];
    char percent_text[16];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, "set") != 0)
        return false;
    if (!command_read_token(&cursor, fan, fan_len))
        return false;
    if (!command_read_token(&cursor, percent_text, sizeof(percent_text)))
        return false;
    if (!command_has_only_trailing_space(cursor))
        return false;

    return command_parse_percent_token(percent_text, percent);
}

bool parse_preset_command(const char *cmd, char *preset_name,
                          size_t preset_name_len)
{
    return command_parse_action(cmd, "preset", preset_name, preset_name_len);
}

bool parse_coolboost_command(const char *cmd, char *action, size_t action_len)
{
    return command_parse_action(cmd, "coolboost", action, action_len);
}

bool parse_fan_mode_command(const char *cmd, char *action, size_t action_len)
{
    return command_parse_action(cmd, "fan-mode", action, action_len);
}

bool parse_profile_command(const char *cmd, char *action, size_t action_len)
{
    return command_parse_action(cmd, "profile", action, action_len);
}

bool parse_power_source_command(const char *cmd, char *action, size_t action_len)
{
    return command_parse_action(cmd, "power-source", action, action_len);
}

bool parse_power_source_auto_command(const char *cmd, char *value,
                                     const size_t value_len)
{
    const char *cursor = cmd;
    char command[32];
    char action[16];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, "power-source") != 0)
        return false;
    if (!command_read_token(&cursor, action, sizeof(action)) ||
        strcmp(action, "auto") != 0)
        return false;
    if (!command_read_token(&cursor, value, value_len))
        return false;
    if (!command_has_only_trailing_space(cursor))
        return false;

    return true;
}

bool parse_gpu_temp_command(const char *cmd, char *action, size_t action_len)
{
    return command_parse_action(cmd, "gpu-temp", action, action_len);
}

bool parse_keyboard_backlight_command(const char *cmd, char *action,
                                      size_t action_len)
{
    return command_parse_action(cmd, "keyboard-backlight", action, action_len);
}

bool parse_keyboard_backlight_set_command(const char *cmd, int *percent)
{
    const char *cursor = cmd;
    char command[32];
    char action[16];
    char percent_text[16];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, "keyboard-backlight") != 0)
        return false;
    if (!command_read_token(&cursor, action, sizeof(action)) ||
        strcmp(action, "set") != 0)
        return false;
    if (!command_read_token(&cursor, percent_text, sizeof(percent_text)))
        return false;
    if (!command_has_only_trailing_space(cursor))
        return false;

    return command_parse_int_token(percent_text, 0, 100, percent);
}

bool parse_keyboard_backlight_timeout_command(const char *cmd, char *action,
                                              const size_t action_len)
{
    const char *cursor = cmd;
    char command[32];
    char subcommand[16];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, "keyboard-backlight") != 0)
        return false;
    if (!command_read_token(&cursor, subcommand, sizeof(subcommand)) ||
        strcmp(subcommand, "timeout") != 0)
        return false;
    if (!command_read_token(&cursor, action, action_len))
        return false;

    return command_has_only_trailing_space(cursor);
}

bool parse_ec_read_command(const char *cmd, int *reg)
{
    const char *cursor = cmd;
    char command[16];
    char reg_text[16];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, "ec-read") != 0)
        return false;
    if (!command_read_token(&cursor, reg_text, sizeof(reg_text)))
        return false;
    if (!command_has_only_trailing_space(cursor))
        return false;

    return command_parse_int_token(reg_text, 0, 255, reg);
}

bool parse_ec_dump_command(const char *cmd, int *start, int *end)
{
    const char *cursor = cmd;
    char command[16];
    char start_text[16];
    char end_text[16];

    if (!command_read_token(&cursor, command, sizeof(command)) ||
        strcmp(command, "ec-dump") != 0)
        return false;
    if (!command_read_token(&cursor, start_text, sizeof(start_text)) ||
        !command_read_token(&cursor, end_text, sizeof(end_text)))
        return false;
    if (!command_has_only_trailing_space(cursor))
        return false;
    if (!command_parse_int_token(start_text, 0, 255, start) ||
        !command_parse_int_token(end_text, 0, 255, end))
        return false;

    return *start <= *end && *end - *start <= 127;
}

bool command_name_is(const char *cmd, const char *name)
{
    const char *cursor = cmd;
    char command[32];

    return command_read_token(&cursor, command, sizeof(command)) &&
           strcmp(command, name) == 0;
}

bool command_is_exact(const char *cmd, const char *name)
{
    const char *cursor = cmd;
    char command[32];

    return command_read_token(&cursor, command, sizeof(command)) &&
           strcmp(command, name) == 0 &&
           command_has_only_trailing_space(cursor);
}
