#ifndef ANS_CLIENT_DOCTOR_UTIL_H
#define ANS_CLIENT_DOCTOR_UTIL_H

/**
 * Print a doctor report section heading.
 *
 * Sections keep command output readable while preserving the exact shell
 * commands used to collect diagnostics.
 */
void doctor_print_section(const char* title);

/**
 * Print the value of one diagnostic file.
 *
 * Missing or unreadable files are reported as command output instead of
 * stopping the doctor report.
 */
void doctor_print_file_value(const char* label, const char* path);

/**
 * Print the operating system name.
 *
 * The helper prefers os-release metadata and falls back cleanly when the file
 * is unavailable.
 */
void doctor_print_os_pretty_name(void);

/**
 * Print resolved command paths for installed binaries.
 *
 * This makes packaging and PATH mistakes visible in doctor output.
 */
void doctor_print_command_paths(void);

/**
 * Print the active model configuration path.
 *
 * The doctor report includes both the symlink and resolved target so install
 * state is clear.
 */
void doctor_print_model_config(void);

/**
 * Run a shell command inside the doctor report.
 *
 * The label is printed before command output so reports remain reproducible
 * and easy to inspect.
 */
void doctor_run_command(const char* label, const char* command);

/**
 * Print control-socket ownership and mode.
 *
 * Permission problems are common during setup, so socket metadata is reported
 * directly.
 */
void doctor_print_socket_permissions(void);

/**
 * Print current user and group membership.
 *
 * Group output confirms whether the current login session can use privileged
 * daemon commands.
 */
void doctor_print_user_groups(void);

#endif
