#ifndef ANS_KEYBOARD_BACKLIGHT_TIMEOUT_H
#define ANS_KEYBOARD_BACKLIGHT_TIMEOUT_H

#include "daemon/types.h"

#include <stdint.h>

void keyboard_backlight_timeout_init(const struct ans_config *cfg,
                                     daemon_runtime_state *runtime);
void keyboard_backlight_timeout_note_manual_set(daemon_runtime_state *runtime,
                                                int percent);
void keyboard_backlight_timeout_tick(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     daemon_runtime_state *runtime,
                                     int64_t now_ms,
                                     int64_t last_activity_ms);

#endif
