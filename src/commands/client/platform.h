#ifndef ANS_CLIENT_PLATFORM_COMMANDS_H
#define ANS_CLIENT_PLATFORM_COMMANDS_H

/**
 * Dispatch CoolBoost commands.
 *
 * Status is formatted locally, while on/off requests are forwarded to the
 * daemon's firmware turbo-mode backend.
 */
int client_handle_coolboost_command(int argc, char** argv);

/**
 * Dispatch firmware fan-mode commands.
 *
 * Accepted actions are validated locally before the daemon updates EC fan-mode
 * registers.
 */
int client_handle_fan_mode_command(int argc, char** argv);

/**
 * Dispatch GPU temperature policy commands.
 *
 * These commands control whether the daemon keeps the GPU sensor live or lets
 * Linux runtime power management suspend it.
 */
int client_handle_gpu_temp_command(int argc, char** argv);

/**
 * Dispatch keyboard backlight commands.
 *
 * The client validates brightness steps and timeout actions before forwarding
 * requests to the daemon.
 */
int client_handle_keyboard_backlight_command(int argc, char** argv);

/**
 * Dispatch AC and battery power-source profile commands.
 *
 * The daemon owns power-source detection and firmware profile writes, while
 * the client owns command-shape validation.
 */
int client_handle_power_source_command(int argc, char** argv);

/**
 * Dispatch platform profile commands.
 *
 * Profile names are forwarded to the daemon so model configuration remains the
 * source of truth.
 */
int client_handle_profile_command(int argc, char** argv);

#endif
