#ifndef ANS_DAEMON_STATE_H
#define ANS_DAEMON_STATE_H

#include "daemon/types.h"

/**
 * Persist daemon control state.
 *
 * Persisted state lets the service restore fan mode, preset, CoolBoost, and
 * optional runtime policies across restarts.
 */
void write_control_state(
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool coolboost_enabled,
    const daemon_runtime_state* runtime
);

/**
 * Restore daemon control state from JSON.
 *
 * Tests and file loading share this parser so restore behavior is covered
 * without touching the real state file.
 */
bool restore_control_state_from_json(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* json
);

/**
 * Restore daemon control state from disk.
 *
 * Missing or invalid state falls back to model defaults rather than blocking
 * daemon startup.
 */
bool restore_control_state(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime
);

#endif
