#include "commands/daemon/registry.h"

#include "daemon/status.h"
#include "control/protocol.h"
#include "commands/daemon/ec.h"
#include "commands/daemon/fan.h"
#include "daemon/capabilities.h"
#include "daemon/feature_status.h"
#include "commands/parser/parser.h"
#include "commands/daemon/control.h"
#include "commands/daemon/platform_handlers.h"

#include <string.h>

/**
 * Handles one top-level daemon command.
 *
 * Registry handlers decide whether a command is read-only, permission-gated,
 * or delegated to a feature-specific command module.
 */
typedef bool (*daemon_registry_handler)(const daemon_command_context* ctx);

/**
 * Maps a top-level daemon command to permission policy and routing.
 *
 * The registry is the only place that should know which command tokens are
 * public entry points on the control socket.
 */
typedef struct
{
    const char* name;
    bool requires_control;
    daemon_registry_handler handler;
} daemon_registry_command;

/**
 * Reply with the standard permission failure.
 *
 * Keeping this in the registry makes every mutating command use the same
 * group-membership guidance.
 */
static bool ensure_control(const daemon_command_context* ctx)
{
    if (ctx->can_control)
        return true;

    control_reply(
        ctx->client,
        "error permission denied: add your user to the %s group and log in again\n",
        ANS_CONTROL_GROUP
    );

    return false;
}

/**
 * Return whether an action command is a status request.
 *
 * Mixed read/write commands use this before the permission check so status
 * remains available to normal users.
 */
static bool command_action_is_status(const char* cmd, bool (*parser)(const char* cmd, char* action, size_t action_len))
{
    char action[16];

    return parser(cmd, action, sizeof(action)) && strcmp(action, "status") == 0;
}

/**
 * Reply with daemon status.
 *
 * Status is a read-only command and must stay available without the control
 * group so panels and scripts can inspect daemon state.
 */
static bool handle_status_registry_command(const daemon_command_context* ctx)
{
    if (!command_is_exact(ctx->cmd, "status"))
    {
        control_reply(ctx->client, "error usage: status\n");

        return true;
    }

    reply_status(ctx->client, ctx->cfg, ctx->states, *ctx->auto_mode, ctx->preset, *ctx->coolboost_enabled);

    return true;
}

/**
 * Reply with available fan presets.
 *
 * Preset listing is read-only model metadata, so it belongs in the registry
 * with the other non-mutating commands.
 */
static bool handle_presets_registry_command(const daemon_command_context* ctx)
{
    if (!command_is_exact(ctx->cmd, "presets"))
    {
        control_reply(ctx->client, "error usage: presets\n");

        return true;
    }

    reply_presets(ctx->client, ctx->cfg);

    return true;
}

/**
 * Reply with the active preset context.
 *
 * This command reports current preset context without writing EC state.
 */
static bool handle_preset_show_registry_command(const daemon_command_context* ctx)
{
    if (!command_is_exact(ctx->cmd, "preset-show"))
    {
        control_reply(ctx->client, "error usage: preset-show\n");

        return true;
    }

    reply_preset_show(ctx->client, ctx->cfg, ctx->states, *ctx->auto_mode, ctx->preset);

    return true;
}

/**
 * Reply with feature capabilities.
 *
 * Capabilities are read-only feature availability details consumed by
 * diagnostics and UI integrations.
 */
static bool handle_capabilities_registry_command(const daemon_command_context* ctx)
{
    if (!command_is_exact(ctx->cmd, "capabilities"))
    {
        control_reply(ctx->client, "error usage: capabilities\n");

        return true;
    }

    reply_capabilities(ctx->client, ctx->cfg, ctx->runtime);

    return true;
}

/**
 * Route EC debug commands.
 *
 * Raw EC reads stay behind the control permission because they expose
 * privileged hardware access.
 */
static bool handle_ec_registry_command(const daemon_command_context* ctx)
{
    return handle_ec_command(ctx->client, ctx->ec, ctx->cmd);
}

/**
 * Route fan control commands.
 *
 * Set, preset, auto, and firmware-auto share the fan command context because
 * they all update the same runtime control state.
 */
static bool handle_fan_registry_command(const daemon_command_context* ctx)
{
    return handle_fan_control_command(
        ctx->client,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        ctx->auto_mode,
        ctx->preset,
        ctx->preset_len,
        ctx->coolboost_enabled,
        ctx->runtime,
        ctx->cmd
    );
}

/**
 * Route CoolBoost commands.
 *
 * The status action is read-only; on/off actions require control permission
 * before reaching the feature handler.
 */
static bool handle_coolboost_registry_command(const daemon_command_context* ctx)
{
    if (command_action_is_status(ctx->cmd, parse_coolboost_command))
    {
        reply_coolboost_status(ctx->client, ctx->cfg, *ctx->coolboost_enabled);

        return true;
    }

    if (!ensure_control(ctx))
        return true;

    return handle_coolboost_command(
        ctx->client,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        *ctx->auto_mode,
        ctx->preset,
        ctx->coolboost_enabled,
        ctx->runtime,
        ctx->cmd
    );
}

/**
 * Route firmware fan-mode commands.
 *
 * Status remains read-only, while firmware fan-mode writes are permission
 * gated by the registry.
 */
static bool handle_fan_mode_registry_command(const daemon_command_context* ctx)
{
    if (command_action_is_status(ctx->cmd, parse_fan_mode_command))
    {
        reply_fan_mode_status(ctx->client, ctx->ec, ctx->cfg);

        return true;
    }

    if (!ensure_control(ctx))
        return true;

    return handle_fan_mode_command(
        ctx->client,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        ctx->auto_mode,
        ctx->preset,
        ctx->preset_len,
        ctx->coolboost_enabled,
        ctx->runtime,
        ctx->cmd
    );
}

/**
 * Route platform profile commands.
 *
 * Profile status is informational, but changing the profile writes firmware
 * state and therefore requires control permission.
 */
static bool handle_profile_registry_command(const daemon_command_context* ctx)
{
    if (command_action_is_status(ctx->cmd, parse_profile_command))
    {
        reply_profile_status(ctx->client, ctx->ec, ctx->cfg);

        return true;
    }

    if (!ensure_control(ctx))
        return true;

    return handle_profile_command(ctx->client, ctx->ec, ctx->cfg, ctx->cmd);
}

/**
 * Route power-source profile commands.
 *
 * Status can be inspected by normal users; apply and auto policy changes are
 * daemon state changes.
 */
static bool handle_power_source_registry_command(const daemon_command_context* ctx)
{
    if (command_action_is_status(ctx->cmd, parse_power_source_command))
    {
        reply_power_source_status(ctx->client, ctx->ec, ctx->cfg, ctx->runtime);

        return true;
    }

    if (!ensure_control(ctx))
        return true;

    return handle_power_source_command(
        ctx->client,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        *ctx->auto_mode,
        ctx->preset,
        *ctx->coolboost_enabled,
        ctx->runtime,
        ctx->cmd
    );
}

/**
 * Route GPU temperature policy commands.
 *
 * Reading the current policy is safe for every client, but changing runtime
 * power control can wake hardware and must be gated.
 */
static bool handle_gpu_temp_registry_command(const daemon_command_context* ctx)
{
    if (command_action_is_status(ctx->cmd, parse_gpu_temp_command))
    {
        reply_gpu_temp_status(ctx->client, ctx->ec, ctx->cfg);

        return true;
    }

    if (!ensure_control(ctx))
        return true;

    return handle_gpu_temp_command(ctx->client, ctx->cmd);
}

/**
 * Route keyboard backlight commands.
 *
 * Status and timeout status are read-only; brightness and timeout changes are
 * persisted daemon state.
 */
static bool handle_keyboard_backlight_registry_command(const daemon_command_context* ctx)
{
    const bool is_backlight_command = command_action_is_status(ctx->cmd, parse_keyboard_backlight_command);
    const bool is_backlight_timeout_command = command_action_is_status(ctx->cmd, parse_keyboard_backlight_timeout_command);

    if (is_backlight_command || is_backlight_timeout_command)
    {
        reply_keyboard_backlight_status(ctx->client, ctx->ec, ctx->cfg, ctx->runtime);

        return true;
    }

    if (!ensure_control(ctx))
        return true;

    return handle_keyboard_backlight_command(
        ctx->client,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        *ctx->auto_mode,
        ctx->preset,
        *ctx->coolboost_enabled,
        ctx->runtime,
        ctx->cmd
    );
}

/**
 * Route daemon lifecycle commands.
 *
 * Resume and stop change daemon or firmware state, so they are always
 * permission-gated top-level commands.
 */
static bool handle_daemon_control_registry_command(const daemon_command_context* ctx)
{
    return handle_daemon_control_command(
        ctx->client,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        *ctx->auto_mode,
        ctx->preset,
        ctx->cmd
    );
}

/**
 * Supported top-level daemon commands.
 *
 * This table is the daemon command surface. Permission policy lives beside
 * each route so read-only commands stay available while mutating commands are
 * consistently group-gated.
 */
static const daemon_registry_command DAEMON_COMMANDS[] = {
    {.name = "status", .requires_control = false, .handler = handle_status_registry_command},
    {.name = "presets", .requires_control = false, .handler = handle_presets_registry_command},
    {.name = "preset-show", .requires_control = false, .handler = handle_preset_show_registry_command},
    {.name = "capabilities", .requires_control = false, .handler = handle_capabilities_registry_command},
    {.name = "coolboost", .requires_control = false, .handler = handle_coolboost_registry_command},
    {.name = "fan-mode", .requires_control = false, .handler = handle_fan_mode_registry_command},
    {.name = "profile", .requires_control = false, .handler = handle_profile_registry_command},
    {.name = "power-source", .requires_control = false, .handler = handle_power_source_registry_command},
    {.name = "gpu-temp", .requires_control = false, .handler = handle_gpu_temp_registry_command},
    {.name = "keyboard-backlight", .requires_control = false, .handler = handle_keyboard_backlight_registry_command},
    {.name = "ec-read", .requires_control = true, .handler = handle_ec_registry_command},
    {.name = "ec-dump", .requires_control = true, .handler = handle_ec_registry_command},
    {.name = "set", .requires_control = true, .handler = handle_fan_registry_command},
    {.name = "preset", .requires_control = true, .handler = handle_fan_registry_command},
    {.name = "auto", .requires_control = true, .handler = handle_fan_registry_command},
    {.name = "firmware-auto", .requires_control = true, .handler = handle_fan_registry_command},
    {.name = "resume", .requires_control = true, .handler = handle_daemon_control_registry_command},
    {.name = "stop", .requires_control = true, .handler = handle_daemon_control_registry_command},
};

/**
 * Find a top-level daemon command entry.
 *
 * Dispatch is intentionally table-driven so adding a command changes one
 * registry instead of another probe chain.
 */
static const daemon_registry_command* find_daemon_command(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(DAEMON_COMMANDS); i++)
    {
        if (strcmp(name, DAEMON_COMMANDS[i].name) == 0)
            return &DAEMON_COMMANDS[i];
    }

    return NULL;
}

/**
 * Dispatch one parsed daemon command.
 *
 * The registry owns top-level command routing so feature handlers do not need
 * to probe commands that belong to other features.
 */
void dispatch_daemon_command(const daemon_command_context* ctx)
{
    char command[32];

    if (!command_first_token(ctx->cmd, command, sizeof(command)))
    {
        control_reply(ctx->client, "error unknown command\n");

        return;
    }

    const daemon_registry_command* entry = find_daemon_command(command);

    if (!entry)
    {
        control_reply(ctx->client, "error unknown command\n");

        return;
    }

    if (entry->requires_control && !ensure_control(ctx))
        return;

    if (!entry->handler(ctx))
        control_reply(ctx->client, "error unknown command\n");
}
