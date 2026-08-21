#include "commands/daemon/platform_handlers.h"

#include "commands/parser/parser.h"
#include "control/protocol.h"
#include "sensors/sensors.h"

#include <stdio.h>
#include <string.h>

bool handle_gpu_temp_command(const int client, const char *cmd)
{
    char action[16];
    const char *policy;
    int changed;

    if (!command_name_is(cmd, "gpu-temp"))
        return false;

    if (!parse_gpu_temp_command(cmd, action, sizeof(action))) {
        control_reply(client, "error usage: gpu-temp status|auto|live\n");
        return true;
    }

    if (strcmp(action, "auto") == 0)
        policy = "auto";
    else if (strcmp(action, "live") == 0)
        policy = "on";
    else {
        control_reply(client, "error usage: gpu-temp status|auto|live\n");
        return true;
    }

    changed = sensor_set_group_power_control("gpu", policy);
    if (changed <= 0) {
        control_reply(client, "error gpu-temp policy write failed policy=%s\n", policy);
        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(stderr, "gpu_temp_policy policy=%s changed=%d\n", policy, changed);
    control_reply(client, "gpu_temp=available policy=%s live=%s changed=%d\n",
            policy, strcmp(policy, "on") == 0 ? "on" : "auto", changed);
    return true;
}
