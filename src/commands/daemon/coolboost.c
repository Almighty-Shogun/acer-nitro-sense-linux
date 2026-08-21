#include "commands/daemon/platform_handlers.h"

#include "control/protocol.h"
#include "commands/parser/parser.h"
#include "daemon/state.h"
#include "platform/control.h"

#include <stdio.h>
#include <string.h>

bool handle_coolboost_command(const int client, struct ec_device *ec,
                              const struct ans_config *cfg,
                              fan_state states[ANS_MAX_FANS],
                              const bool auto_mode, const char *preset,
                              bool *coolboost_enabled,
                              const daemon_runtime_state *runtime,
                              const char *cmd)
{
    char action[16];

    if (!command_name_is(cmd, "coolboost"))
        return false;

    if (!parse_coolboost_command(cmd, action, sizeof(action)) ||
        (strcmp(action, "on") != 0 && strcmp(action, "off") != 0)) {
        control_reply(client, "error usage: coolboost on|off|status\n");
        return true;
    }

    if (!cfg->fan_modes.available) {
        control_reply(client, "error coolboost unavailable for this model\n");
        return true;
    }

    const bool enabled = strcmp(action, "on") == 0;
    const bool applied = enabled ? apply_coolboost(ec, cfg, states, true) :
        (firmware_auto_mode(auto_mode, preset) ?
            apply_firmware_auto_fan_mode(ec, cfg) :
            apply_coolboost(ec, cfg, states, false));

    if (!applied) {
        control_reply(client, "error coolboost write failed\n");
        return true;
    }

    *coolboost_enabled = enabled;
    write_control_state(cfg, states, auto_mode, preset, *coolboost_enabled,
                        runtime);
    if (!daemon_quiet_logs)
        fprintf(stderr, "coolboost_change enabled=%d backend=fan-mode-turbo\n",
                enabled ? 1 : 0);
    control_reply(client, "coolboost=%s\n", enabled ? "on" : "off");
    return true;
}
