#ifndef ANS_KEYBOARD_BACKLIGHT_TIMEOUT_H
#define ANS_KEYBOARD_BACKLIGHT_TIMEOUT_H

#include "daemon/types.h"

#include <stdint.h>

/**
 * Initialize keyboard backlight timeout runtime state.
 *
 * Startup state comes from the model profile and restored daemon state.
 */
void keyboard_backlight_timeout_init(const struct ans_config* cfg, daemon_runtime_state* runtime);

/**
 * Record a manual keyboard backlight brightness change.
 *
 * The timeout controller restores this level after activity wakes a dimmed
 * keyboard.
 */
void keyboard_backlight_timeout_note_manual_set(daemon_runtime_state* runtime, int percent);

/**
 * Advance keyboard backlight timeout state.
 *
 * The tick compares current time with last input activity and writes EC
 * brightness only when the timeout state changes.
 */
void keyboard_backlight_timeout_tick(
    struct ec_device* ec,
    const struct ans_config* cfg,
    daemon_runtime_state* runtime,
    int64_t now_ms,
    int64_t last_activity_ms
);

#endif
