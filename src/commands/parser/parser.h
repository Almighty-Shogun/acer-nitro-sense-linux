#ifndef ANS_COMMAND_H
#define ANS_COMMAND_H

#include <stddef.h>
#include <stdbool.h>

#define COMMAND_ARRAY_LEN(values) (sizeof(values) / sizeof((values)[0]))

/**
 * Return whether a token is in an allowed string set.
 *
 * Command handlers use this to keep action validation table-driven instead of
 * repeating chains of string comparisons.
 */
bool command_token_in(const char *token, const char *const *values, size_t len);

/**
 * Return whether a value is in an allowed integer set.
 *
 * Numeric command arguments such as keyboard brightness steps can then be
 * checked against the exact model-supported values.
 */
bool command_int_in(int value, const int *values, size_t len);

/**
 * Read the first token from a command line.
 *
 * Dispatchers use this once to select a command table entry before handing the
 * full command line to the owning feature handler.
 */
bool command_first_token(const char *cmd, char *token, size_t token_len);

/**
 * Read the second token from a command line.
 *
 * Feature dispatchers use this to select a subcommand table entry after the
 * top-level daemon registry has selected the owning command.
 */
bool command_second_token(const char *cmd, char *token, size_t token_len);

/**
 * Parse a one-action command and validate the action against an allowed set.
 *
 * Feature handlers use this when a command has a closed action list, such as
 * `fan-mode auto|manual|turbo`.
 */
bool command_parse_enum_action(
    const char *cmd,
    const char *expected_command,
    const char *const *values,
    size_t values_len,
    char *action,
    size_t action_len
);

/**
 * Parse a manual fan-speed command.
 *
 * The command must match `set cpu|gpu|all 1-100`. The parsed fan id and
 * percent are returned only when the full command shape is valid.
 */
bool parse_set_command(const char *cmd, char *fan, size_t fan_len, int *percent);

/**
 * Parse a preset application command.
 *
 * The command must match `preset NAME`. The parser only validates the command
 * shape; the daemon config decides whether the preset name exists.
 */
bool parse_preset_command(const char *cmd, char *preset_name, size_t preset_name_len);

/**
 * Parse a CoolBoost action command.
 *
 * The command must match `coolboost ACTION`. The caller validates ACTION so
 * status and mutating actions can be permission-gated differently.
 */
bool parse_coolboost_command(const char *cmd, char *action, size_t action_len);

/**
 * Parse a firmware fan-mode command.
 *
 * The command must match `fan-mode ACTION`. The parser leaves action validity
 * to the fan-mode command table.
 */
bool parse_fan_mode_command(const char *cmd, char *action, size_t action_len);

/**
 * Parse a platform profile command.
 *
 * The command must match `profile ACTION`. Firmware profile availability and
 * profile names are validated by the platform layer.
 */
bool parse_profile_command(const char *cmd, char *action, size_t action_len);

/**
 * Parse a power-source command action.
 *
 * The command must match `power-source ACTION`. Status is read-only while
 * apply and auto changes require control permission.
 */
bool parse_power_source_command(const char *cmd, char *action, size_t action_len);

/**
 * Parse a power-source auto toggle command.
 *
 * The command must match `power-source auto VALUE`, where VALUE is later
 * validated as an on/off token.
 */
bool parse_power_source_auto_command(const char *cmd, char *value, size_t value_len);

/**
 * Parse a GPU temperature policy command.
 *
 * The command must match `gpu-temp ACTION`. The daemon maps the action to the
 * Linux runtime-power policy used by the GPU sensor.
 */
bool parse_gpu_temp_command(const char *cmd, char *action, size_t action_len);

/**
 * Parse a keyboard backlight action command.
 *
 * The command must match `keyboard-backlight ACTION`. The caller uses ACTION
 * to route status, brightness, and timeout behavior.
 */
bool parse_keyboard_backlight_command(const char *cmd, char *action, size_t action_len);

/**
 * Parse a keyboard backlight brightness command.
 *
 * The command must match `keyboard-backlight set PERCENT`. The parser checks
 * the numeric range; the command layer checks supported brightness steps.
 */
bool parse_keyboard_backlight_set_command(const char *cmd, int *percent);

/**
 * Parse a keyboard backlight timeout command.
 *
 * The command must match `keyboard-backlight timeout ACTION`, allowing timeout
 * status and toggle behavior to share one parser.
 */
bool parse_keyboard_backlight_timeout_command(const char *cmd, char *action, size_t action_len);

/**
 * Parse a raw EC register read command.
 *
 * The command must match `ec-read REG`, where REG is bounded to one EC byte
 * address.
 */
bool parse_ec_read_command(const char *cmd, int *reg);

/**
 * Parse a raw EC register dump command.
 *
 * The command must match `ec-dump START END`. Range-size policy is enforced by
 * the handler so parser and command safety stay separate.
 */
bool parse_ec_dump_command(const char *cmd, int *start, int *end);

/**
 * Return whether a command line is exactly one command token.
 *
 * Lifecycle and read-only commands use this to reject trailing arguments
 * before taking action.
 */
bool command_is_exact(const char *cmd, const char *name);

#endif
