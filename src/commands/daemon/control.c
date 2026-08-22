#include "commands/daemon/control.h"

#include "commands/parser/parser.h"
#include "control/protocol.h"
#include "fan/control.h"
#include "platform/control.h"

#include <stdio.h>

bool handle_daemon_control_command(const int client, struct ec_device *ec,
                                   const struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS],
                                   const bool auto_mode, const char *preset,
                                   const char *cmd)
{
    if (command_is_exact(cmd, "resume")) {
        apply_init_writes(ec, cfg);
        apply_sensor_power_control(cfg, "on");
        if (firmware_auto_mode(auto_mode, preset))
            apply_firmware_auto_fan_mode(ec, cfg);
        else
            apply_current_control_state(ec, cfg, states);
        if (!daemon_quiet_logs)
            fprintf(stderr, "resume_reapply mode=%s preset=%s\n",
                    control_mode(auto_mode, preset), preset);
        control_reply(client, "resume=ok mode=%s preset=%s\n",
                      control_mode(auto_mode, preset), preset);

        return true;
    }

    if (command_is_exact(cmd, "stop")) {
        daemon_running = 0;
        control_reply(client, "stop=ok reset=firmware\n");

        return true;
    }

    return false;
}
