#ifndef ANS_COMMAND_INTERNAL_H
#define ANS_COMMAND_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

const char *command_skip_space(const char *p);
bool command_read_token(const char **cursor, char *out, size_t out_len);
bool command_has_only_trailing_space(const char *p);
bool command_parse_percent_token(const char *text, int *percent);
bool command_parse_int_token(const char *text, int min_value, int max_value,
                             int *value);
bool command_parse_action(const char *cmd, const char *expected_command,
                          char *action, size_t action_len);
bool command_parse_subaction(const char *cmd, const char *expected_command,
                             const char *expected_action, char *value,
                             size_t value_len);
bool command_parse_int_action(const char *cmd, const char *expected_command,
                              int min_value, int max_value, int *value);
bool command_parse_int_subaction(const char *cmd, const char *expected_command,
                                 const char *expected_action, int min_value,
                                 int max_value, int *value);

#endif
