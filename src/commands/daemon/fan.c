#include "commands/daemon/fan.h"

#include "fan/control.h"
#include "config/config.h"
#include "control/protocol.h"
#include "platform/control.h"
#include "commands/parser/parser.h"
#include "commands/daemon/fan_context.h"

#include <stdio.h>
#include <string.h>

/**
 * Handles one daemon fan-control command.
 *
 * Fan commands share mutable control state, so handlers receive a compact
 * context object instead of a long argument list.
 */
typedef bool (*fan_control_handler)(const fan_command_context* ctx, const char* cmd);

/**
 * Maps a fan-control command name to its daemon handler.
 *
 * This table owns only commands that change or restore fan-control mode. Other
 * platform features are routed by their own command modules.
 */
typedef struct
{
    const char* name;
    fan_control_handler handler;
} fan_control_command;

/**
 * Apply a manual fan percentage.
 *
 * Manual percentages switch the firmware into daemon-owned fan control before
 * writing the requested fan registers.
 */
static bool handle_set_command(const fan_command_context* ctx, const char* cmd)
{
    int percent;
    char fan[32];

    if (!parse_set_command(cmd, fan, sizeof(fan), &percent))
    {
        control_reply(ctx->client, "error usage: set cpu|gpu|all 1-100\n");

        return true;
    }

    if (!fan_command_prepare_daemon_control(ctx))
        return true;

    fan_command_set_control_mode(ctx, false, "manual");

    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=manual fan=%s requested=%d\n", fan, percent);

    const int changed = set_one(ctx->ec, ctx->cfg, ctx->states, fan, percent);

    if (changed == 0)
    {
        control_reply(ctx->client, "error unknown fan: %s\n", fan);

        return true;
    }

    fan_command_write_state(ctx);
    control_reply(ctx->client, "mode=manual fan=%s requested=%d\n", fan, percent);

    return true;
}

/**
 * Apply a configured fan preset.
 *
 * Presets are model-defined percentage pairs that keep user-facing fan modes
 * stable while the underlying EC values remain profile-specific.
 */
static bool handle_preset_command(const fan_command_context* ctx, const char* cmd)
{
    char preset_name[32];

    if (!parse_preset_command(cmd, preset_name, sizeof(preset_name)))
    {
        control_reply(ctx->client, "error usage: preset NAME\n");

        return true;
    }

    const struct preset_config* p = config_find_preset(ctx->cfg, preset_name);

    if (!p)
    {
        control_reply(ctx->client, "error unknown preset\n");

        return true;
    }

    if (!fan_command_prepare_daemon_control(ctx))
        return true;

    fan_command_set_control_mode(ctx, false, p->id);

    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=preset preset=%s cpu=%d gpu=%d\n", p->id, p->cpu, p->gpu);

    apply_preset(ctx->ec, ctx->cfg, ctx->states, p->id);
    fan_command_write_state(ctx);

    control_reply(ctx->client, "mode=preset preset=%s cpu=%d gpu=%d\n", p->id, p->cpu, p->gpu);

    return true;
}

/**
 * Enable the daemon automatic fan curve.
 *
 * Auto mode starts from each fan's configured reset speed and lets the daemon
 * loop adjust percentages as temperatures change.
 */
static bool handle_auto_command(const fan_command_context* ctx, const char* cmd)
{
    if (!command_is_exact(cmd, "auto"))
    {
        control_reply(ctx->client, "error usage: auto\n");

        return true;
    }

    if (!fan_command_prepare_daemon_control(ctx))
        return true;

    fan_command_set_control_mode(ctx, true, "auto");

    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=auto preset=auto\n");

    for (int i = 0; i < ctx->cfg->fan_len; i++)
    {
        set_fan_percent(
            ctx->ec,
            ctx->cfg,
            &ctx->cfg->fans[i],
            &ctx->states[i],
            ctx->cfg->fans[i].reset_speed,
            global_safety_reason(ctx->cfg, ctx->states)
        );
    }

    fan_command_write_state(ctx);
    control_reply(ctx->client, "mode=auto preset=auto\n");

    return true;
}

/**
 * Return fan control to firmware automatic mode.
 *
 * Firmware-auto writes the model fan-mode registers and records that the
 * daemon should report firmware-owned percentages instead of manual values.
 */
static bool handle_firmware_auto_command(const fan_command_context* ctx, const char* cmd)
{
    if (!command_is_exact(cmd, "firmware-auto"))
    {
        control_reply(ctx->client, "error usage: firmware-auto\n");

        return true;
    }

    if (!ctx->cfg->fan_modes.available)
    {
        control_reply(ctx->client, "error firmware-auto unavailable for this model\n");

        return true;
    }

    if (!apply_firmware_auto_fan_mode(ctx->ec, ctx->cfg))
    {
        control_reply(ctx->client, "error fan-mode write failed\n");

        return true;
    }

    fan_command_set_control_mode(ctx, false, FIRMWARE_AUTO_PRESET);

    *ctx->coolboost_enabled = false;

    fan_command_write_state(ctx);

    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=%s preset=%s\n", FIRMWARE_AUTO_PRESET, FIRMWARE_AUTO_PRESET);

    control_reply(ctx->client, "mode=%s preset=%s\n", FIRMWARE_AUTO_PRESET, FIRMWARE_AUTO_PRESET);

    return true;
}

/**
 * Supported daemon fan-control commands.
 *
 * These are the commands that change the active fan-control mode or restore
 * it. Read-only fan metadata is handled by the daemon registry.
 */
static const fan_control_command FAN_CONTROL_COMMANDS[] = {
    {.name = "set", .handler = handle_set_command},
    {.name = "preset", .handler = handle_preset_command},
    {.name = "auto", .handler = handle_auto_command},
    {.name = "firmware-auto", .handler = handle_firmware_auto_command},
};

/**
 * Find a fan-control command entry.
 *
 * The top-level daemon registry already knows this is a fan command. This
 * local table keeps fan-specific routing next to the fan behavior.
 */
static const fan_control_command* find_fan_control_command(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(FAN_CONTROL_COMMANDS); i++)
    {
        if (strcmp(name, FAN_CONTROL_COMMANDS[i].name) == 0)
            return &FAN_CONTROL_COMMANDS[i];
    }

    return NULL;
}

/**
 * Dispatch a daemon fan-control command.
 *
 * The registry already selected the fan-control module, so this function only
 * routes between fan-specific commands and reports unknown fan commands.
 */
bool handle_fan_control_command(
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
    const fan_command_context ctx = {
        .client = client,
        .ec = ec,
        .cfg = cfg,
        .states = states,
        .auto_mode = auto_mode,
        .preset = preset,
        .preset_len = preset_len,
        .coolboost_enabled = coolboost_enabled,
        .runtime = runtime,
    };

    char command[32];

    if (!command_first_token(cmd, command, sizeof(command)))
    {
        control_reply(client, "error unknown command\n");

        return true;
    }

    const fan_control_command* entry = find_fan_control_command(command);

    if (entry)
        return entry->handler(&ctx, cmd);

    control_reply(client, "error unknown command\n");

    return true;
}
