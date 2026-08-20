#ifndef ANS_KEYBOARD_BACKLIGHT_H
#define ANS_KEYBOARD_BACKLIGHT_H

#include <stdbool.h>

struct ans_config;
struct ec_device;

struct keyboard_backlight_status {
    bool available;
    char name[128];
    char path[512];
    char backend[32];
    int reg;
    int brightness;
    int max_brightness;
    int percent;
};

bool keyboard_backlight_read(struct keyboard_backlight_status *status);
bool keyboard_backlight_read_any(struct ec_device *ec, const struct ans_config *cfg,
                                 struct keyboard_backlight_status *status);
bool keyboard_backlight_set_percent(struct ec_device *ec,
                                    const struct ans_config *cfg,
                                    int percent,
                                    struct keyboard_backlight_status *status);
const char *keyboard_backlight_reason(const struct keyboard_backlight_status *status);

#endif
