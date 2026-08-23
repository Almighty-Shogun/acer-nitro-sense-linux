#ifndef ANS_DAEMON_UNIT_PLATFORM_CASES_H
#define ANS_DAEMON_UNIT_PLATFORM_CASES_H

#include "daemon/types.h"

/**
 * Run platform status command tests.
 *
 * These cases verify read-only status output for optional platform features.
 */
int unit_run_platform_status_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

/**
 * Run platform fan-mode command tests.
 *
 * These cases verify firmware fan-mode writes and CoolBoost state transitions.
 */
int unit_run_platform_fan_mode_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

/**
 * Run power-source profile command tests.
 *
 * These cases verify status, auto-apply, and explicit profile application
 * behavior.
 */
int unit_run_platform_power_source_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

/**
 * Run keyboard backlight command tests.
 *
 * These cases verify brightness steps, timeout policy, and unsupported-model
 * replies.
 */
int unit_run_platform_keyboard_backlight_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

#endif
