#ifndef ANS_KEYBOARD_BACKLIGHT_INTERNAL_H
#define ANS_KEYBOARD_BACKLIGHT_INTERNAL_H

#include "keyboard/backlight.h"

#include "config/types.h"

void keyboard_backlight_init_status(struct keyboard_backlight_status *status);
int keyboard_backlight_percent_from_range(int value, int min_value, int max_value);
int keyboard_backlight_value_from_percent(const struct keyboard_backlight_config *cfg,
                                          int percent);
bool keyboard_backlight_read_sysfs(struct keyboard_backlight_status *status);
bool keyboard_backlight_read_ec(struct ec_device *ec, const struct ans_config *cfg,
                                struct keyboard_backlight_status *status);

#endif
