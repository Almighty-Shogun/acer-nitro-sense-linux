#ifndef ANS_COMMANDS_CLIENT_BUILTIN_H
#define ANS_COMMANDS_CLIENT_BUILTIN_H

/**
 * Print the client status command.
 *
 * Status is read-only and supports human-readable output, JSON output, and
 * optional temperature-unit conversion.
 */
int client_handle_status_command(int argc, char** argv);

/**
 * Start the daemon service.
 *
 * The command delegates to systemd after validating that no extra arguments
 * were supplied.
 */
int client_handle_start_command(int argc, char** argv);

/**
 * Restart the daemon service.
 *
 * Restart is intentionally systemd-backed so the installed service receives
 * the same lifecycle handling as a normal administrator command.
 */
int client_handle_restart_command(int argc, char** argv);

/**
 * Stop the daemon service.
 *
 * The client first asks the daemon to shut down cleanly, then falls back to
 * systemd when the control socket is unavailable.
 */
int client_handle_stop_command(int argc, char** argv);

/**
 * Enable the daemon-managed automatic fan curve.
 *
 * This command switches control from fixed percentages or presets back to the
 * daemon fan curve.
 */
int client_handle_auto_command(int argc, char** argv);

/**
 * Enable the firmware automatic fan mode.
 *
 * Firmware-auto returns fan control to the model's firmware fan-mode registers
 * instead of the daemon curve.
 */
int client_handle_firmware_auto_command(int argc, char** argv);

/**
 * List fan presets.
 *
 * Preset metadata comes from the active model profile and is safe to query
 * without changing EC state.
 */
int client_handle_presets_command(int argc, char** argv);

/**
 * Print feature capability reporting.
 *
 * Capabilities describe which fan, platform, keyboard, and sensor features are
 * available for the active model.
 */
int client_handle_capabilities_command(int argc, char** argv);

/**
 * Reapply daemon state after resume.
 *
 * Resume reapplies EC initialization and the persisted fan-control state after
 * suspend or manual recovery.
 */
int client_handle_resume_command(int argc, char** argv);

/**
 * Validate the active model.
 *
 * Validation exercises model configuration and hardware access without
 * starting the long-running daemon loop.
 */
int client_handle_validate_command(int argc, char** argv);

/**
 * Print diagnostic reporting.
 *
 * Doctor output gathers install, permission, sensor, and platform state for
 * support and model bring-up.
 */
int client_handle_doctor_command(int argc, char** argv);

#endif
