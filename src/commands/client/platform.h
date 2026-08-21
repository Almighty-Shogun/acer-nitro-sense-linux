#ifndef ANS_CLIENT_PLATFORM_COMMANDS_H
#define ANS_CLIENT_PLATFORM_COMMANDS_H

int client_handle_coolboost_command(int argc, char **argv);
int client_handle_fan_mode_command(int argc, char **argv);
int client_handle_gpu_temp_command(int argc, char **argv);
int client_handle_keyboard_backlight_command(int argc, char **argv);
int client_handle_power_source_command(int argc, char **argv);
int client_handle_profile_command(int argc, char **argv);

#endif
