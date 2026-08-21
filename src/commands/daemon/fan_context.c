#include "commands/daemon/fan_context.h"

#include "control/protocol.h"
#include "daemon/state.h"
#include "fan/control.h"
#include "platform/control.h"
#include "util/string.h"

static void disable_coolboost_if_needed(struct ec_device *ec,
                                        const struct ans_config *cfg,
                                        fan_state states[ANS_MAX_FANS],
                                        bool *coolboost_enabled)
{
    if (!*coolboost_enabled)
        return;

    apply_coolboost(ec, cfg, states, false);
    *coolboost_enabled = false;
}

static bool switch_to_daemon_control(const int client, struct ec_device *ec,
                                     const struct ans_config *cfg)
{
    if (apply_daemon_control_fan_mode(ec, cfg))
        return true;

    control_reply(client, "error fan-mode write failed\n");
    return false;
}

bool fan_command_prepare_daemon_control(const fan_command_context *ctx)
{
    disable_coolboost_if_needed(ctx->ec, ctx->cfg, ctx->states,
                                ctx->coolboost_enabled);
    return switch_to_daemon_control(ctx->client, ctx->ec, ctx->cfg);
}

void fan_command_set_control_mode(const fan_command_context *ctx,
                                  const bool auto_enabled,
                                  const char *preset_name)
{
    *ctx->auto_mode = auto_enabled;
    string_copy(ctx->preset, ctx->preset_len, preset_name);
}

void fan_command_write_state(const fan_command_context *ctx)
{
    write_control_state(ctx->cfg, ctx->states, *ctx->auto_mode, ctx->preset,
                        *ctx->coolboost_enabled, ctx->runtime);
}
