#include "commands/daemon/platform_handlers.h"

#include "daemon/state.h"
#include "control/protocol.h"
#include "platform/control.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Describes one coolboost toggle action.
 *
 * CoolBoost status is handled by the registry as a read-only action, so this
 * table only contains the mutating on/off states.
 */
typedef struct {
    const char* name;
    bool enabled;
} coolboost_action;

/**
 * Supported mutating CoolBoost actions.
 *
 * Status is answered by the daemon registry. Only actions that change fan
 * mode state are routed through this table.
 */
static const coolboost_action COOLBOOST_ACTIONS[] = {
    {.name = "on", .enabled = true},
    {.name = "off", .enabled = false},
};

/**
 * Find the coolboost action matching a parsed command token.
 *
 * Unknown tokens return NULL so the handler can emit the same usage string for
 * every invalid mutating action.
 */
static const coolboost_action* find_coolboost_action(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(COOLBOOST_ACTIONS); i++)
    {
        if (strcmp(name, COOLBOOST_ACTIONS[i].name) == 0)
            return &COOLBOOST_ACTIONS[i];
    }

    return NULL;
}

/**
 * Apply a CoolBoost action.
 *
 * CoolBoost is implemented as the firmware turbo fan mode. Disabling it
 * restores either firmware-auto or daemon-owned fan control depending on the
 * state that was active before turbo was requested.
 */
bool handle_coolboost_command(
    const int client,
    struct ec_device *ec,
    const struct ans_config *cfg,
    fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char *preset,
    bool *coolboost_enabled,
    const daemon_runtime_state *runtime,
    const char *cmd
)
{
    char action[16];

    if (!parse_coolboost_command(cmd, action, sizeof(action)))
    {
        control_reply(client, "error usage: coolboost on|off|status\n");

        return true;
    }

    const coolboost_action* entry = find_coolboost_action(action);

    if (!entry)
    {
        control_reply(client, "error usage: coolboost on|off|status\n");

        return true;
    }

    if (!cfg->fan_modes.available) {
        control_reply(client, "error coolboost unavailable for this model\n");

        return true;
    }

    const bool applied = entry->enabled
        ? apply_coolboost(ec, cfg, states, true)
        : firmware_auto_mode(auto_mode, preset)
        ? apply_firmware_auto_fan_mode(ec, cfg)
        : apply_coolboost(ec, cfg, states, false);

    if (!applied) {
        control_reply(client, "error coolboost write failed\n");

        return true;
    }

    *coolboost_enabled = entry->enabled;
    write_control_state(cfg, states, auto_mode, preset, *coolboost_enabled, runtime);

    if (!daemon_quiet_logs)
        fprintf(stderr, "coolboost_change enabled=%d backend=fan-mode-turbo\n", entry->enabled ? 1 : 0);

    control_reply(client, "coolboost=%s\n", entry->name);

    return true;
}
