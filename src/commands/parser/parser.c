#include "commands/parser/parser.h"
#include "commands/parser/internal.h"

#include <string.h>

/**
 * Parse set.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_set_command(const char* cmd, char* fan, const size_t fan_len, int* percent)
{
    const char* cursor = cmd;

    char command[16];
    char percent_text[16];

    if (!command_read_token(&cursor, command, sizeof(command)) || strcmp(command, "set") != 0)
        return false;

    if (!command_read_token(&cursor, fan, fan_len))
        return false;

    if (!command_read_token(&cursor, percent_text, sizeof(percent_text)))
        return false;

    if (!command_has_only_trailing_space(cursor))
        return false;

    return command_parse_percent_token(percent_text, percent);
}

/**
 * Parse preset.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_preset_command(const char* cmd, char* preset_name, const size_t preset_name_len)
{
    return command_parse_action(cmd, "preset", preset_name, preset_name_len);
}

/**
 * Parse coolboost.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_coolboost_command(const char* cmd, char* action, const size_t action_len)
{
    return command_parse_action(cmd, "coolboost", action, action_len);
}

/**
 * Parse mode.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_fan_mode_command(const char* cmd, char* action, const size_t action_len)
{
    return command_parse_action(cmd, "fan-mode", action, action_len);
}

/**
 * Parse profile command.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_profile_command(const char* cmd, char* action, const size_t action_len)
{
    return command_parse_action(cmd, "profile", action, action_len);
}

/**
 * Parse power source command.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_power_source_command(const char* cmd, char* action, const size_t action_len)
{
    return command_parse_action(cmd, "power-source", action, action_len);
}

/**
 * Parse auto.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_power_source_auto_command(const char* cmd, char* value, const size_t value_len)
{
    return command_parse_sub_action(cmd, "power-source", "auto", value, value_len);
}

/**
 * Parse temp.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_gpu_temp_command(const char* cmd, char* action, const size_t action_len)
{
    return command_parse_action(cmd, "gpu-temp", action, action_len);
}

/**
 * Parse keyboard backlight command.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_keyboard_backlight_command(const char* cmd, char* action, const size_t action_len)
{
    return command_parse_action(cmd, "keyboard-backlight", action, action_len);
}

/**
 * Parse set.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_keyboard_backlight_set_command(const char* cmd, int* percent)
{
    return command_parse_int_sub_action(cmd, "keyboard-backlight", "set", 0, 100, percent);
}

/**
 * Parse timeout.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_keyboard_backlight_timeout_command(const char* cmd, char* action, const size_t action_len)
{
    return command_parse_sub_action(cmd, "keyboard-backlight", "timeout", action, action_len);
}

/**
 * Parse read.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_ec_read_command(const char* cmd, int* reg)
{
    return command_parse_int_action(cmd, "ec-read", 0, 255, reg);
}

/**
 * Parse dump.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool parse_ec_dump_command(const char* cmd, int* start, int* end)
{
    const char* cursor = cmd;

    char command[16];
    char start_text[16];
    char end_text[16];

    if (!command_read_token(&cursor, command, sizeof(command)) || strcmp(command, "ec-dump") != 0)
        return false;

    if (!command_read_token(&cursor, start_text, sizeof(start_text)) || !command_read_token(&cursor, end_text, sizeof(end_text)))
        return false;

    if (!command_has_only_trailing_space(cursor))
        return false;

    if (!command_parse_int_token(start_text, 0, 255, start) || !command_parse_int_token(end_text, 0, 255, end))
        return false;

    return *start <= *end && *end - *start <= 127;
}

/**
 * Return whether exact.
 *
 * Command handlers rely on this narrow grammar check before mutating daemon
 * state. Keeping parsing isolated makes malformed socket input fail
 * consistently.
 */
bool command_is_exact(const char* cmd, const char* name)
{
    const char* cursor = cmd;
    char command[32];

    return command_read_token(&cursor, command, sizeof(command))
        && strcmp(command, name) == 0
        && command_has_only_trailing_space(cursor);
}
