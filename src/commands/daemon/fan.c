#include "commands/daemon/fan.h"

#include "commands/daemon/fan_context.h"
#include "commands/parser/parser.h"
#include "config/config.h"
#include "control/protocol.h"
#include "fan/control.h"
#include "platform/control.h"

#include <stdio.h>
#include <string.h>

static bool handle_set_command(const fan_command_context *ctx, const char *cmd)
{
    char fan[32];
    int percent;

    if (!command_name_is(cmd, "set"))
        return false;

    if (!parse_set_command(cmd, fan, sizeof(fan), &percent)) {
        control_reply(ctx->client, "error usage: set cpu|gpu|all 1-100\n");
        return true;
    }

    if (!fan_command_prepare_daemon_control(ctx))
        return true;

    fan_command_set_control_mode(ctx, false, "manual");
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=manual fan=%s requested=%d\n", fan, percent);

    const int changed = set_one(ctx->ec, ctx->cfg, ctx->states, fan, percent);

    if (changed == 0) {
        control_reply(ctx->client, "error unknown fan: %s\n", fan);
        return true;
    }

    fan_command_write_state(ctx);
    control_reply(ctx->client, "mode=manual fan=%s requested=%d\n", fan, percent);
    return true;
}

static bool handle_preset_command(const fan_command_context *ctx,
                                  const char *cmd)
{
    char preset_name[32];

    if (!command_name_is(cmd, "preset"))
        return false;

    if (!parse_preset_command(cmd, preset_name, sizeof(preset_name))) {
        control_reply(ctx->client, "error usage: preset NAME\n");
        return true;
    }

    const struct preset_config *p = config_find_preset(ctx->cfg, preset_name);

    if (!p) {
        control_reply(ctx->client, "error unknown preset\n");
        return true;
    }

    if (!fan_command_prepare_daemon_control(ctx))
        return true;

    fan_command_set_control_mode(ctx, false, p->id);
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=preset preset=%s cpu=%d gpu=%d\n",
                p->id, p->cpu, p->gpu);

    apply_preset(ctx->ec, ctx->cfg, ctx->states, p->id);
    fan_command_write_state(ctx);
    control_reply(ctx->client, "mode=preset preset=%s cpu=%d gpu=%d\n",
                  p->id, p->cpu, p->gpu);
    return true;
}

static bool handle_auto_command(const fan_command_context *ctx, const char *cmd)
{
    if (!command_is_exact(cmd, "auto"))
        return false;

    if (!fan_command_prepare_daemon_control(ctx))
        return true;

    fan_command_set_control_mode(ctx, true, "auto");
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=auto preset=auto\n");

    for (int i = 0; i < ctx->cfg->fan_len; i++) {
        set_fan_percent(ctx->ec, ctx->cfg, &ctx->cfg->fans[i], &ctx->states[i],
                        ctx->cfg->fans[i].reset_speed,
                        global_safety_reason(ctx->cfg, ctx->states));
    }

    fan_command_write_state(ctx);
    control_reply(ctx->client, "mode=auto preset=auto\n");
    return true;
}

static bool handle_firmware_auto_command(const fan_command_context *ctx,
                                         const char *cmd)
{
    if (!command_is_exact(cmd, "firmware-auto"))
        return false;

    if (!ctx->cfg->fan_modes.available) {
        control_reply(ctx->client,
                      "error firmware-auto unavailable for this model\n");
        return true;
    }

    if (!apply_firmware_auto_fan_mode(ctx->ec, ctx->cfg)) {
        control_reply(ctx->client, "error fan-mode write failed\n");
        return true;
    }

    fan_command_set_control_mode(ctx, false, FIRMWARE_AUTO_PRESET);
    *ctx->coolboost_enabled = false;
    fan_command_write_state(ctx);
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=%s preset=%s\n", FIRMWARE_AUTO_PRESET,
                FIRMWARE_AUTO_PRESET);
    control_reply(ctx->client, "mode=%s preset=%s\n", FIRMWARE_AUTO_PRESET,
                  FIRMWARE_AUTO_PRESET);
    return true;
}

bool handle_fan_control_command(const int client, struct ec_device *ec,
                                const struct ans_config *cfg,
                                fan_state states[ANS_MAX_FANS],
                                bool *auto_mode, char *preset,
                                const size_t preset_len,
                                bool *coolboost_enabled,
                                const daemon_runtime_state *runtime,
                                const char *cmd)
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

    if (handle_set_command(&ctx, cmd))
        return true;
    if (handle_preset_command(&ctx, cmd))
        return true;
    if (handle_auto_command(&ctx, cmd))
        return true;
    return handle_firmware_auto_command(&ctx, cmd);
}
