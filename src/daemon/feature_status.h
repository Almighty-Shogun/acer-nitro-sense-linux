#ifndef ANS_DAEMON_FEATURE_STATUS_H
#define ANS_DAEMON_FEATURE_STATUS_H

#include "daemon/types.h"

/**
 * Reply with CoolBoost availability and state.
 *
 * The reply distinguishes unsupported models from supported models where the
 * toggle is currently off.
 */
void reply_coolboost_status(int client, const struct ans_config* cfg, bool coolboost_enabled);

/**
 * Reply with firmware fan-mode status.
 *
 * The reply includes raw CPU and GPU mode values so model debugging remains
 * possible.
 */
void reply_fan_mode_status(int client, struct ec_device* ec, const struct ans_config* cfg);

/**
 * Reply with platform profile status.
 *
 * The reply maps the current EC value back to a configured profile name when
 * possible.
 */
void reply_profile_status(int client, struct ec_device* ec, const struct ans_config* cfg);

/**
 * Reply with AC/battery profile policy status.
 *
 * The reply shows source detection, auto-apply state, configured targets, and
 * the profile that would be applied now.
 */
void reply_power_source_status(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const daemon_runtime_state* runtime
);

/**
 * Reply with GPU temperature power-policy status.
 *
 * The reply reports whether live GPU temperatures are available and which
 * runtime-power policy is active.
 */
void reply_gpu_temp_status(int client, struct ec_device* ec, const struct ans_config* cfg);

/**
 * Reply with keyboard backlight status.
 *
 * The reply includes brightness, backend, timeout, and unsupported-state
 * details for diagnostics and UI integrations.
 */
void reply_keyboard_backlight_status(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const daemon_runtime_state* runtime
);

#endif
