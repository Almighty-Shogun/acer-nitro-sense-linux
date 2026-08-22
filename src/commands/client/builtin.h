#ifndef ANS_COMMANDS_CLIENT_BUILTIN_H
#define ANS_COMMANDS_CLIENT_BUILTIN_H

int client_handle_status_command(int argc, char **argv);
int client_handle_start_command(int argc, char **argv);
int client_handle_restart_command(int argc, char **argv);
int client_handle_stop_command(int argc, char **argv);
int client_handle_auto_command(int argc, char **argv);
int client_handle_firmware_auto_command(int argc, char **argv);
int client_handle_presets_command(int argc, char **argv);
int client_handle_capabilities_command(int argc, char **argv);
int client_handle_resume_command(int argc, char **argv);
int client_handle_validate_command(int argc, char **argv);
int client_handle_doctor_command(int argc, char **argv);

#endif
