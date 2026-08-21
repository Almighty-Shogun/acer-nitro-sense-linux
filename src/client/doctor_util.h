#ifndef ANS_CLIENT_DOCTOR_UTIL_H
#define ANS_CLIENT_DOCTOR_UTIL_H

void doctor_print_section(const char *title);
void doctor_print_file_value(const char *label, const char *path);
void doctor_print_os_pretty_name(void);
void doctor_print_command_paths(void);
void doctor_print_model_config(void);
void doctor_run_command(const char *label, const char *command);
void doctor_print_socket_permissions(void);
void doctor_print_user_groups(void);

#endif
