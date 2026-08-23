#ifndef ANS_DAEMON_PLATFORM_HANDLERS_H
#define ANS_DAEMON_PLATFORM_HANDLERS_H

#include "daemon/types.h"

/**
 * Apply CoolBoost toggle commands.
 *
 * CoolBoost is mapped to firmware turbo fan mode when the active model exposes
 * fan-mode registers.
 */
bool handle_coolboost_command(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool* coolboost_enabled,
    const daemon_runtime_state* runtime,
    const char* cmd
);

/**
 * Apply firmware fan-mode commands.
 *
 * Fan-mode changes write model-specific EC values and update persisted daemon
 * state when the selected mode changes ownership.
 */
bool handle_fan_mode_command(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    const daemon_runtime_state* runtime,
    const char* cmd
);

/**
 * Apply platform profile commands.
 *
 * Platform profiles map user-facing names such as quiet or performance to EC
 * values defined by the active model profile.
 */
bool handle_profile_command(int client, struct ec_device* ec, const struct ans_config* cfg, const char* cmd);

/**
 * Apply power-source profile policy commands.
 *
 * These commands apply or persist the AC/battery profile policy while keeping
 * power-source detection inside the daemon.
 */
bool handle_power_source_command(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
);

/**
 * Apply GPU temperature power-policy commands.
 *
 * The handler switches the GPU sensor between runtime-power auto behavior and
 * always-live temperature reporting.
 */
bool handle_gpu_temp_command(int client, const char* cmd);

/**
 * Apply keyboard backlight commands.
 *
 * Keyboard commands are optional because not every Nitro model exposes the
 * same EC registers. The handler reports unsupported hardware explicitly
 * instead of silently ignoring writes.
 */
bool handle_keyboard_backlight_command(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
);

#endif
