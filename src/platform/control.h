#ifndef ANS_PLATFORM_CONTROL_H
#define ANS_PLATFORM_CONTROL_H

#include "daemon/types.h"

extern const char FIRMWARE_AUTO_PRESET[];

/**
 * Return whether the daemon is in firmware-auto mode.
 *
 * Firmware-auto is represented by a normal preset string so status and state
 * persistence can describe firmware ownership without a separate mode enum.
 */
bool firmware_auto_mode(bool auto_mode, const char* preset);

/**
 * Resolve the active fan control mode.
 *
 * This converts the daemon's auto flag and preset string into the public
 * status mode shown by the CLI and integrations.
 */
const char* control_mode(bool auto_mode, const char* preset);

/**
 * Resolve a firmware fan-mode register value to a name.
 *
 * CPU and GPU fan-mode registers can use different values, so the caller
 * identifies which side is being interpreted.
 */
const char* fan_mode_value_name(const struct fan_mode_config* fan_modes, bool cpu, int value);

/**
 * Apply a named firmware fan mode.
 *
 * The active model profile maps the mode name to the EC values written for CPU
 * and GPU fans.
 */
bool apply_fan_mode(struct ec_device* ec, const struct ans_config* cfg, const char* mode);

/**
 * Read current firmware fan-mode register values.
 *
 * Callers can format the raw values through `fan_mode_value_name()` when the
 * model exposes fan-mode registers.
 */
bool read_fan_mode(struct ec_device* ec, const struct ans_config* cfg, int* cpu_value, int* gpu_value);

/**
 * Apply or clear CoolBoost through firmware fan mode.
 *
 * Enabling CoolBoost writes turbo mode. Disabling it restores the current
 * daemon-controlled fan percentages unless firmware-auto owns the fans.
 */
bool apply_coolboost(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS], bool enabled);

/**
 * Switch firmware fan mode to daemon-controlled manual writes.
 *
 * Manual or preset fan percentages must take ownership before EC speed
 * registers are written.
 */
bool apply_daemon_control_fan_mode(struct ec_device* ec, const struct ans_config* cfg);

/**
 * Switch firmware fan mode to automatic control.
 *
 * This returns speed decisions to the laptop firmware and is used by the
 * firmware-auto preset.
 */
bool apply_firmware_auto_fan_mode(struct ec_device* ec, const struct ans_config* cfg);

/**
 * Apply a named platform profile.
 *
 * Profile names are resolved through the model profile before the EC register
 * is written.
 */
bool apply_platform_profile(struct ec_device* ec, const struct ans_config* cfg, const char* profile);

/**
 * Resolve a platform profile EC value to a name.
 *
 * Unknown values are reported as unavailable so diagnostics do not guess at
 * firmware state.
 */
const char* platform_profile_value_name(const struct ans_config* cfg, int value);

/**
 * Read the current platform profile EC value.
 *
 * The caller is responsible for formatting the returned raw value for status
 * output.
 */
bool read_platform_profile(struct ec_device* ec, const struct ans_config* cfg, int* value);

/**
 * Apply configured sensor runtime-power policy.
 *
 * This is used for GPU temperature visibility where Linux may suspend the
 * device and make hwmon reads temporarily unavailable.
 */
void apply_sensor_power_control(const struct ans_config* cfg, const char* control);

#endif
