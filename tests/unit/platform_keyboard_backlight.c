#include "unit/platform_cases.h"

#include "unit/helpers.h"

#include <stdio.h>
#include <string.h>

int unit_run_platform_keyboard_backlight_commands(
    struct ec_device *ec,
    const struct ans_config *cfg,
    fan_state states[ANS_MAX_FANS],
    bool *auto_mode,
    char *preset,
    const size_t preset_len,
    bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    if (unit_execute_command("keyboard-backlight status\n", ec, cfg,
                                 states, auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "keyboard_backlight=available") ||
        !strstr(reply, "backend=ec") ||
        !strstr(reply, "register=0x31") ||
        !strstr(reply, "timeout=off")) {
        fprintf(stderr, "unit-test failed: keyboard-backlight ec status command path\n");
        failures++;
    }

    if (unit_execute_command("keyboard-backlight set 75\n", ec, cfg,
                                 states, auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "keyboard_backlight=available") ||
        !strstr(reply, "brightness=3") ||
        !strstr(reply, "percent=75")) {
        fprintf(stderr, "unit-test failed: keyboard-backlight ec set command path\n");
        failures++;
    }

    if (unit_execute_command("keyboard-backlight timeout on\n", ec, cfg,
                                 states, auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "keyboard_backlight_timeout=on") ||
        !strstr(reply, "timeout_seconds=30")) {
        fprintf(stderr, "unit-test failed: keyboard-backlight timeout on command path\n");
        failures++;
    }

    if (unit_execute_command("keyboard-backlight timeout off\n", ec, cfg,
                                 states, auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "keyboard_backlight_timeout=off")) {
        fprintf(stderr, "unit-test failed: keyboard-backlight timeout off command path\n");
        failures++;
    }

    return failures;
}
