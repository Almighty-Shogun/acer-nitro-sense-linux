#include "commands/daemon/platform_handlers.h"

#include "sensors/sensors.h"
#include "control/protocol.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Describes how a GPU temperature policy action maps to sysfs state.
 *
 * The daemon exposes user-facing policy names while Linux expects power
 * control values, so the mapping stays explicit here.
 */
typedef struct
{
    const char* name;
    const char* policy;
    const char* live;
} gpu_temp_action;

/**
 * Supported GPU temperature power policies.
 *
 * The public action names are stable CLI terms; the policy values are the
 * sysfs power-control values written to matching GPU devices.
 */
static const gpu_temp_action GPU_TEMP_ACTIONS[] = {
    {.name = "auto", .policy = "auto", .live = "auto"},
    {.name = "live", .policy = "on", .live = "on"},
};

/**
 * Find the GPU temperature policy action matching a parsed command token.
 *
 * Returning NULL lets the caller produce the same usage error for unsupported
 * actions and malformed requests.
 */
static const gpu_temp_action* find_gpu_temp_action(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(GPU_TEMP_ACTIONS); i++)
    {
        if (strcmp(name, GPU_TEMP_ACTIONS[i].name) == 0)
            return &GPU_TEMP_ACTIONS[i];
    }

    return NULL;
}

/**
 * Apply a GPU temperature visibility policy.
 *
 * The live policy keeps the NVIDIA device awake so hwmon can report a current
 * temperature; auto returns runtime power management to the kernel.
 */
bool handle_gpu_temp_command(const int client, const char* cmd)
{
    char action[16];

    if (!parse_gpu_temp_command(cmd, action, sizeof(action)))
    {
        control_reply(client, "error usage: gpu-temp status|auto|live\n");

        return true;
    }

    const gpu_temp_action* entry = find_gpu_temp_action(action);

    if (!entry)
    {
        control_reply(client, "error usage: gpu-temp status|auto|live\n");

        return true;
    }

    const int changed = sensor_set_group_power_control("gpu", entry->policy);

    if (changed <= 0)
    {
        control_reply(client, "error gpu-temp policy write failed policy=%s\n", entry->policy);

        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(stderr, "gpu_temp_policy policy=%s changed=%d\n", entry->policy, changed);

    control_reply(
        client,
        "gpu_temp=available policy=%s live=%s changed=%d\n",
        entry->policy,
        entry->live,
        changed
    );

    return true;
}
