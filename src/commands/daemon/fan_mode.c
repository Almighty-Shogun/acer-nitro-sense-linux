#include "commands/daemon/platform_handlers.h"

#include "fan/control.h"
#include "util/string.h"
#include "daemon/state.h"
#include "control/protocol.h"
#include "platform/control.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Describes the runtime state changes caused by a fan-mode action.
 *
 * Firmware fan modes do not all mean the same thing for daemon state. The
 * table keeps the EC write action and persisted control-mode side effects
 * visible together.
 */
typedef struct
{
    const char* name;
    bool coolboost_enabled;
    bool updates_control_mode;
    const char* preset;
    bool apply_current_control;
} fan_mode_action;

/**
 * Supported firmware fan-mode actions.
 *
 * Each action records both the firmware mode and the daemon state changes that
 * must be persisted after the EC write succeeds.
 */
static const fan_mode_action FAN_MODE_ACTIONS[] = {
    {.name = "auto", .coolboost_enabled = false, .updates_control_mode = true, .preset = NULL, .apply_current_control = false},
    {.name = "manual", .coolboost_enabled = false, .updates_control_mode = true, .preset = "manual", .apply_current_control = true},
    {.name = "turbo", .coolboost_enabled = true, .updates_control_mode = false, .preset = NULL, .apply_current_control = false},
};

/**
 * Find the fan-mode action matching a parsed command token.
 *
 * Status is handled before this module is called, so a missing entry is an
 * invalid mutating fan-mode action.
 */
static const fan_mode_action* find_fan_mode_action(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(FAN_MODE_ACTIONS); i++)
    {
        if (strcmp(name, FAN_MODE_ACTIONS[i].name) == 0)
            return &FAN_MODE_ACTIONS[i];
    }

    return NULL;
}

/**
 * Apply a firmware fan-mode action.
 *
 * Auto and manual become normal persisted control modes, while turbo is
 * treated as the CoolBoost-compatible temporary fan mode.
 */
bool handle_fan_mode_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled,
    const daemon_runtime_state* runtime,
    const char* cmd
)
{
    char action[16];

    if (!parse_fan_mode_command(cmd, action, sizeof(action)))
    {
        control_reply(client, "error usage: fan-mode status|auto|manual|turbo\n");

        return true;
    }

    const fan_mode_action* entry = find_fan_mode_action(action);

    if (!entry)
    {
        control_reply(client, "error usage: fan-mode status|auto|manual|turbo\n");

        return true;
    }

    if (!cfg->fan_modes.available)
    {
        control_reply(client, "error fan modes unavailable for this model\n");

        return true;
    }

    if (!apply_fan_mode(ec, cfg, action))
    {
        control_reply(client, "error fan-mode write failed\n");

        return true;
    }

    *coolboost_enabled = entry->coolboost_enabled;

    if (entry->updates_control_mode)
    {
        *auto_mode = false;

        const char* preset_name = entry->preset ? entry->preset : FIRMWARE_AUTO_PRESET;

        string_copy(preset, preset_len, preset_name);
    }

    if (entry->apply_current_control)
        apply_current_control_state(ec, cfg, states);

    write_control_state(cfg, states, *auto_mode, preset, *coolboost_enabled, runtime);

    if (!daemon_quiet_logs)
        fprintf(
            stderr,
            "fan_mode_change mode=%s cpu_register=0x%02x gpu_register=0x%02x\n",
            action,
            cfg->fan_modes.cpu_reg,
            cfg->fan_modes.gpu_reg
        );

    control_reply(client, "fan_mode=%s\n", action);

    return true;
}
