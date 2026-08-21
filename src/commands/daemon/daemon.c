#include "commands/daemon/daemon.h"
#include "commands/parser/parser.h"
#include "control/protocol.h"
#include "control/socket.h"
#include "commands/daemon/ec.h"
#include "commands/daemon/fan.h"
#include "commands/daemon/platform.h"
#include "commands/daemon/readonly.h"
#include "fan/control.h"
#include "platform/control.h"

#include <stdio.h>

void execute_command(const int client, struct ec_device *ec, const struct ans_config *cfg,
                     fan_state states[ANS_MAX_FANS], bool *auto_mode,
                     char *preset, const size_t preset_len,
                     bool *coolboost_enabled,
                     daemon_runtime_state *runtime,
                     const char *cmd, const bool can_control)
{
    if (handle_readonly_command(client, ec, cfg, states, *auto_mode, preset,
                                *coolboost_enabled, runtime, cmd))
        return;

    if (!can_control) {
        control_reply(client, "error permission denied: add your user to the %s group and log in again\n", ANS_CONTROL_GROUP);

        return;
    }

    if (handle_ec_command(client, ec, cmd))
        return;

    if (handle_fan_control_command(client, ec, cfg, states, auto_mode, preset,
                                   preset_len, coolboost_enabled, runtime, cmd))
        return;

    if (handle_platform_command(client, ec, cfg, states, auto_mode, preset,
                                preset_len, coolboost_enabled, runtime, cmd))
        return;

    if (command_is_exact(cmd, "resume")) {
        apply_init_writes(ec, cfg);
        apply_sensor_power_control(cfg, "on");
        if (firmware_auto_mode(*auto_mode, preset))
            apply_firmware_auto_fan_mode(ec, cfg);
        else
            apply_current_control_state(ec, cfg, states);
        if (!daemon_quiet_logs)
            fprintf(stderr, "resume_reapply mode=%s preset=%s\n", control_mode(*auto_mode, preset), preset);
        control_reply(client, "resume=ok mode=%s preset=%s\n", control_mode(*auto_mode, preset), preset);

        return;
    }

    if (command_is_exact(cmd, "stop")) {
        daemon_running = 0;
        control_reply(client, "stop=ok reset=firmware\n");

        return;
    }

    control_reply(client, "error unknown command\n");
}

void handle_client(const int client, struct ec_device *ec, const struct ans_config *cfg,
                   fan_state states[ANS_MAX_FANS], bool *auto_mode,
                   char *preset, const size_t preset_len,
                   bool *coolboost_enabled,
                   daemon_runtime_state *runtime)
{
    char cmd[256];
    const int read_result = control_read_command(client, cmd, sizeof(cmd));

    if (read_result < 0) {
        control_reply(client, "error invalid command\n");
        return;
    }

    execute_command(client, ec, cfg, states, auto_mode, preset, preset_len,
                    coolboost_enabled, runtime, cmd, client_can_control(client));
}
