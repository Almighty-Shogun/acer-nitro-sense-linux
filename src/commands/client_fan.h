#ifndef ANS_CLIENT_FAN_COMMANDS_H
#define ANS_CLIENT_FAN_COMMANDS_H

int client_handle_ec_command(int argc, char **argv);
int client_handle_preset_command(int argc, char **argv);
int client_handle_set_command(int argc, char **argv);

#endif
