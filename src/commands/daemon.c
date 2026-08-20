#include "commands/daemon.h"
#include "commands/parser.h"
#include "control/socket.h"
#include "commands/ec.h"
#include "commands/fan.h"
#include "commands/platform.h"
#include "commands/readonly.h"
#include "fan/control.h"
#include "platform/control.h"

#include <stdio.h>
#include <unistd.h>

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
        dprintf(client, "error permission denied: add your user to the %s group and log in again\n", ANS_CONTROL_GROUP);

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
        dprintf(client, "resume=ok mode=%s preset=%s\n", control_mode(*auto_mode, preset), preset);

        return;
    }

    if (command_is_exact(cmd, "stop")) {
        daemon_running = 0;
        dprintf(client, "stop=ok reset=firmware\n");

        return;
    }

    dprintf(client, "error unknown command\n");
}

void handle_client(const int client, struct ec_device *ec, const struct ans_config *cfg,
                   fan_state states[ANS_MAX_FANS], bool *auto_mode,
                   char *preset, const size_t preset_len,
                   bool *coolboost_enabled,
                   daemon_runtime_state *runtime)
{
    char cmd[256];
    const ssize_t n = read(client, cmd, sizeof(cmd) - 1);

    if (n <= 0)
        return;

    cmd[n] = '\0';
    execute_command(client, ec, cfg, states, auto_mode, preset, preset_len,
                    coolboost_enabled, runtime, cmd, client_can_control(client));
}
