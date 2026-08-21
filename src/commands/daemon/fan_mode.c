#include "commands/daemon/platform_handlers.h"

#include "control/protocol.h"
#include "commands/parser/parser.h"
#include "daemon/state.h"
#include "fan/control.h"
#include "platform/control.h"
#include "util/string.h"

#include <stdio.h>
#include <string.h>

bool handle_fan_mode_command(const int client, struct ec_device *ec,
                             const struct ans_config *cfg,
                             fan_state states[ANS_MAX_FANS],
                             bool *auto_mode, char *preset,
                             const size_t preset_len,
                             bool *coolboost_enabled,
                             const daemon_runtime_state *runtime,
                             const char *cmd)
{
    char action[16];

    if (!command_name_is(cmd, "fan-mode"))
        return false;

    if (!parse_fan_mode_command(cmd, action, sizeof(action)) ||
        (strcmp(action, "auto") != 0 &&
         strcmp(action, "manual") != 0 &&
         strcmp(action, "turbo") != 0)) {
        control_reply(client, "error usage: fan-mode status|auto|manual|turbo\n");
        return true;
    }

    if (!cfg->fan_modes.available) {
        control_reply(client, "error fan modes unavailable for this model\n");
        return true;
    }

    if (!apply_fan_mode(ec, cfg, action)) {
        control_reply(client, "error fan-mode write failed\n");
        return true;
    }

    *coolboost_enabled = strcmp(action, "turbo") == 0;
    if (strcmp(action, "auto") == 0) {
        *auto_mode = false;
        string_copy(preset, preset_len, FIRMWARE_AUTO_PRESET);
    } else if (strcmp(action, "manual") == 0) {
        *auto_mode = false;
        string_copy(preset, preset_len, "manual");
        apply_current_control_state(ec, cfg, states);
    }
    write_control_state(cfg, states, *auto_mode, preset, *coolboost_enabled,
                        runtime);

    if (!daemon_quiet_logs)
        fprintf(stderr, "fan_mode_change mode=%s cpu_register=0x%02x gpu_register=0x%02x\n",
                action, cfg->fan_modes.cpu_reg, cfg->fan_modes.gpu_reg);
    control_reply(client, "fan_mode=%s\n", action);
    return true;
}
