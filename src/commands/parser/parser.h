#ifndef ANS_COMMAND_H
#define ANS_COMMAND_H

#include <stdbool.h>
#include <stddef.h>

bool parse_set_command(const char *cmd, char *fan, size_t fan_len, int *percent);
bool parse_preset_command(const char *cmd, char *preset_name,
                          size_t preset_name_len);
bool parse_coolboost_command(const char *cmd, char *action, size_t action_len);
bool parse_fan_mode_command(const char *cmd, char *action, size_t action_len);
bool parse_profile_command(const char *cmd, char *action, size_t action_len);
bool parse_power_source_command(const char *cmd, char *action, size_t action_len);
bool parse_power_source_auto_command(const char *cmd, char *value, size_t value_len);
bool parse_gpu_temp_command(const char *cmd, char *action, size_t action_len);
bool parse_keyboard_backlight_command(const char *cmd, char *action,
                                      size_t action_len);
bool parse_keyboard_backlight_set_command(const char *cmd, int *percent);
bool parse_keyboard_backlight_timeout_command(const char *cmd, char *action,
                                              size_t action_len);
bool parse_ec_read_command(const char *cmd, int *reg);
bool parse_ec_dump_command(const char *cmd, int *start, int *end);
bool command_name_is(const char *cmd, const char *name);
bool command_is_exact(const char *cmd, const char *name);

#endif
