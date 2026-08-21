#include "commands/daemon/platform_handlers.h"

#include "control/protocol.h"
#include "commands/parser/parser.h"
#include "daemon/state.h"
#include "keyboard/backlight.h"
#include "keyboard/backlight_timeout.h"

#include <stdio.h>
#include <string.h>

bool handle_keyboard_backlight_command(const int client, struct ec_device *ec,
                                       const struct ans_config *cfg,
                                       const fan_state states[ANS_MAX_FANS],
                                       const bool auto_mode,
                                       const char *preset,
                                       const bool coolboost_enabled,
                                       daemon_runtime_state *runtime,
                                       const char *cmd)
{
    int percent;
    char action[16];
    struct keyboard_backlight_status status;

    if (!command_name_is(cmd, "keyboard-backlight"))
        return false;

    if (parse_keyboard_backlight_timeout_command(cmd, action, sizeof(action))) {
        if (!cfg->keyboard_backlight.timeout_supported) {
            control_reply(client,
                    "error keyboard-backlight timeout unavailable for this model\n");
            return true;
        }

        if (strcmp(action, "on") != 0 && strcmp(action, "off") != 0) {
            control_reply(client,
                    "error usage: keyboard-backlight timeout status|on|off\n");
            return true;
        }

        runtime->keyboard_backlight_timeout_enabled =
            strcmp(action, "on") == 0;
        if (!runtime->keyboard_backlight_timeout_enabled)
            runtime->keyboard_backlight_timed_off = false;
        write_control_state(cfg, states, auto_mode, preset, coolboost_enabled,
                            runtime);

        control_reply(client,
                "keyboard_backlight_timeout=%s timeout_seconds=%d backend=input-activity\n",
                runtime->keyboard_backlight_timeout_enabled ? "on" : "off",
                cfg->keyboard_backlight.timeout_seconds);
        return true;
    }

    if (!parse_keyboard_backlight_set_command(cmd, &percent)) {
        control_reply(client,
                "error usage: keyboard-backlight status|set 0-100|timeout status|on|off\n");
        return true;
    }

    if (!cfg->keyboard_backlight.available) {
        control_reply(client,
                "error keyboard-backlight unavailable: no EC backend configured for this model\n");
        return true;
    }

    if (!keyboard_backlight_set_percent(ec, cfg, percent, &status)) {
        control_reply(client,
                "error keyboard-backlight write failed register=0x%02x percent=%d\n",
                cfg->keyboard_backlight.reg, percent);
        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(stderr,
                "keyboard_backlight_change backend=ec register=0x%02x brightness=%d percent=%d\n",
                cfg->keyboard_backlight.reg, status.brightness, status.percent);

    keyboard_backlight_timeout_note_manual_set(runtime, status.percent);
    control_reply(client,
            "keyboard_backlight=available backend=ec register=0x%02x brightness=%d max_brightness=%d percent=%d\n",
            cfg->keyboard_backlight.reg, status.brightness,
            status.max_brightness, status.percent);
    return true;
}
