#ifndef ANS_COMMAND_INTERNAL_H
#define ANS_COMMAND_INTERNAL_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Skip command whitespace.
 *
 * Parsers use this before reading tokens so commands accept harmless spacing
 * without accepting malformed token sequences.
 */
const char* command_skip_space(const char* p);

/**
 * Read one command token.
 *
 * The cursor is advanced past the token only when the output buffer can hold
 * the complete token.
 */
bool command_read_token(const char** cursor, char* out, size_t out_len);

/**
 * Return whether only whitespace remains after parsing.
 *
 * Command parsers use this as their final check so trailing garbage is
 * rejected consistently.
 */
bool command_has_only_trailing_space(const char* p);

/**
 * Parse a percent token accepted by fan commands.
 *
 * The value must be numeric and within the daemon's 1-100 percent fan-control
 * range.
 */
bool command_parse_percent_token(const char* text, int* percent);

/**
 * Parse a bounded integer token.
 *
 * This helper rejects partial conversions and values outside the caller's
 * accepted range.
 */
bool command_parse_int_token(const char* text, int min_value, int max_value, int* value);

/**
 * Parse a command with one action token.
 *
 * The command name must match exactly and no third token may be present.
 */
bool command_parse_action(const char* cmd, const char* expected_command, char* action, size_t action_len);

/**
 * Parse a command with an action and sub-action.
 *
 * This is used for nested commands such as `power-source auto on`, where the
 * first action is fixed and the final value is returned.
 */
bool command_parse_sub_action(
    const char* cmd,
    const char* expected_command,
    const char* expected_action,
    char* value,
    size_t value_len
);

/**
 * Parse an integer command action.
 *
 * The command name must match and the integer action must be fully consumed
 * inside the supplied range.
 */
bool command_parse_int_action(const char* cmd, const char* expected_command, int min_value, int max_value, int* value);

/**
 * Parse an integer command sub-action.
 *
 * This handles nested commands whose final token is a bounded numeric value.
 */
bool command_parse_int_sub_action(
    const char* cmd,
    const char* expected_command,
    const char* expected_action,
    int min_value,
    int max_value,
    int* value
);

#endif
