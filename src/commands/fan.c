#include "commands/fan.h"

#include "commands/parser.h"
#include "config/config.h"
#include "control/protocol.h"
#include "daemon/state.h"
#include "fan/control.h"
#include "platform/control.h"
#include "util/string.h"

#include <stdio.h>
#include <string.h>

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

static bool prepare_daemon_control(const int client, struct ec_device *ec,
                                   const struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS],
                                   bool *coolboost_enabled)
{
    disable_coolboost_if_needed(ec, cfg, states, coolboost_enabled);
    return switch_to_daemon_control(client, ec, cfg);
}

static void set_control_mode(bool *auto_mode, char *preset,
                             const size_t preset_len,
                             const bool auto_enabled,
                             const char *preset_name)
{
    *auto_mode = auto_enabled;
    string_copy(preset, preset_len, preset_name);
}

static bool handle_set_command(const int client, struct ec_device *ec,
                               const struct ans_config *cfg,
                               fan_state states[ANS_MAX_FANS],
                               bool *auto_mode, char *preset,
                               const size_t preset_len,
                               bool *coolboost_enabled,
                               const daemon_runtime_state *runtime,
                               const char *cmd)
{
    char fan[32];
    int percent;

    if (!command_name_is(cmd, "set"))
        return false;

    if (!parse_set_command(cmd, fan, sizeof(fan), &percent)) {
        control_reply(client, "error usage: set cpu|gpu|all 1-100\n");
        return true;
    }

    if (!prepare_daemon_control(client, ec, cfg, states, coolboost_enabled))
        return true;

    set_control_mode(auto_mode, preset, preset_len, false, "manual");
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=manual fan=%s requested=%d\n", fan, percent);

    const int changed = set_one(ec, cfg, states, fan, percent);

    if (changed == 0) {
        control_reply(client, "error unknown fan: %s\n", fan);
        return true;
    }

    write_control_state(cfg, states, *auto_mode, preset, *coolboost_enabled,
                        runtime);
    control_reply(client, "mode=manual fan=%s requested=%d\n", fan, percent);
    return true;
}

static bool handle_preset_command(const int client, struct ec_device *ec,
                                  const struct ans_config *cfg,
                                  fan_state states[ANS_MAX_FANS],
                                  bool *auto_mode, char *preset,
                                  const size_t preset_len,
                                  bool *coolboost_enabled,
                                  const daemon_runtime_state *runtime,
                                  const char *cmd)
{
    char preset_name[32];

    if (!command_name_is(cmd, "preset"))
        return false;

    if (!parse_preset_command(cmd, preset_name, sizeof(preset_name))) {
        control_reply(client, "error usage: preset NAME\n");
        return true;
    }

    const struct preset_config *p = config_find_preset(cfg, preset_name);

    if (!p) {
        control_reply(client, "error unknown preset\n");
        return true;
    }

    if (!prepare_daemon_control(client, ec, cfg, states, coolboost_enabled))
        return true;

    set_control_mode(auto_mode, preset, preset_len, false, p->id);
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=preset preset=%s cpu=%d gpu=%d\n",
                p->id, p->cpu, p->gpu);

    apply_preset(ec, cfg, states, p->id);
    write_control_state(cfg, states, *auto_mode, preset, *coolboost_enabled,
                        runtime);
    control_reply(client, "mode=preset preset=%s cpu=%d gpu=%d\n", p->id, p->cpu, p->gpu);
    return true;
}

static bool handle_auto_command(const int client, struct ec_device *ec,
                                const struct ans_config *cfg,
                                fan_state states[ANS_MAX_FANS],
                                bool *auto_mode, char *preset,
                                const size_t preset_len,
                                bool *coolboost_enabled,
                                const daemon_runtime_state *runtime,
                                const char *cmd)
{
    if (!command_is_exact(cmd, "auto"))
        return false;

    if (!prepare_daemon_control(client, ec, cfg, states, coolboost_enabled))
        return true;

    set_control_mode(auto_mode, preset, preset_len, true, "auto");
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=auto preset=auto\n");

    for (int i = 0; i < cfg->fan_len; i++) {
        set_fan_percent(ec, cfg, &cfg->fans[i], &states[i],
                        cfg->fans[i].reset_speed,
                        global_safety_reason(cfg, states));
    }

    write_control_state(cfg, states, *auto_mode, preset, *coolboost_enabled,
                        runtime);
    control_reply(client, "mode=auto preset=auto\n");
    return true;
}

static bool handle_firmware_auto_command(const int client, struct ec_device *ec,
                                         const struct ans_config *cfg,
                                         fan_state states[ANS_MAX_FANS],
                                         bool *auto_mode, char *preset,
                                         const size_t preset_len,
                                         bool *coolboost_enabled,
                                         const daemon_runtime_state *runtime,
                                         const char *cmd)
{
    if (!command_is_exact(cmd, "firmware-auto"))
        return false;

    if (!cfg->fan_modes.available) {
        control_reply(client, "error firmware-auto unavailable for this model\n");
        return true;
    }

    if (!apply_firmware_auto_fan_mode(ec, cfg)) {
        control_reply(client, "error fan-mode write failed\n");
        return true;
    }

    set_control_mode(auto_mode, preset, preset_len, false, FIRMWARE_AUTO_PRESET);
    *coolboost_enabled = false;
    write_control_state(cfg, states, *auto_mode, preset, *coolboost_enabled,
                        runtime);
    if (!daemon_quiet_logs)
        fprintf(stderr, "mode_change mode=%s preset=%s\n", FIRMWARE_AUTO_PRESET,
                FIRMWARE_AUTO_PRESET);
    control_reply(client, "mode=%s preset=%s\n", FIRMWARE_AUTO_PRESET, FIRMWARE_AUTO_PRESET);
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
    if (handle_set_command(client, ec, cfg, states, auto_mode, preset,
                           preset_len, coolboost_enabled, runtime, cmd))
        return true;
    if (handle_preset_command(client, ec, cfg, states, auto_mode, preset,
                              preset_len, coolboost_enabled, runtime, cmd))
        return true;
    if (handle_auto_command(client, ec, cfg, states, auto_mode, preset,
                            preset_len, coolboost_enabled, runtime, cmd))
        return true;
    return handle_firmware_auto_command(client, ec, cfg, states, auto_mode,
                                        preset, preset_len, coolboost_enabled,
                                        runtime, cmd);
}
