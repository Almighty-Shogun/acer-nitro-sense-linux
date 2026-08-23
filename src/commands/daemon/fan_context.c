#include "commands/daemon/fan_context.h"

#include "util/string.h"
#include "daemon/state.h"
#include "control/protocol.h"
#include "platform/control.h"

/**
 * Disable CoolBoost before daemon-owned fan writes.
 *
 * Manual, preset, and daemon-auto modes must clear the turbo fan mode before
 * writing normal fan percentages.
 */
static void disable_coolboost_if_needed(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* coolboost_enabled
)
{
    if (!*coolboost_enabled) return;

    apply_coolboost(ec, cfg, states, false);

    *coolboost_enabled = false;
}

/**
 * Switch firmware fan mode to daemon-owned control.
 *
 * The caller can then write percentage registers knowing the firmware is no
 * longer in automatic fan mode.
 */
static bool switch_to_daemon_control(const int client, struct ec_device* ec, const struct ans_config* cfg)
{
    if (apply_daemon_control_fan_mode(ec, cfg))
        return true;

    control_reply(client, "error fan-mode write failed\n");

    return false;
}

/**
 * Prepare fan commands for daemon-owned control.
 *
 * This clears turbo mode and switches the firmware fan mode before any manual,
 * preset, or daemon-auto percentage write is attempted.
 */
bool fan_command_prepare_daemon_control(const fan_command_context* ctx)
{
    disable_coolboost_if_needed(ctx->ec, ctx->cfg, ctx->states, ctx->coolboost_enabled);

    return switch_to_daemon_control(ctx->client, ctx->ec, ctx->cfg);
}

/**
 * Record the active daemon control mode.
 *
 * The fan handlers update both the auto flag and preset label through this
 * helper before persisting state.
 */
void fan_command_set_control_mode(const fan_command_context* ctx, const bool auto_enabled, const char* preset_name)
{
    *ctx->auto_mode = auto_enabled;

    string_copy(ctx->preset, ctx->preset_len, preset_name);
}

/**
 * Persist the current fan command state.
 *
 * Persisting through one helper keeps fan, preset, and CoolBoost state writes
 * consistent across fan-control commands.
 */
void fan_command_write_state(const fan_command_context* ctx)
{
    write_control_state(ctx->cfg, ctx->states, *ctx->auto_mode, ctx->preset, *ctx->coolboost_enabled, ctx->runtime);
}
