#include "commands/daemon/platform.h"

#include "commands/daemon/platform_handlers.h"

bool handle_platform_command(const int client, struct ec_device *ec,
                             const struct ans_config *cfg,
                             fan_state states[ANS_MAX_FANS],
                             bool *auto_mode, char *preset,
                             const size_t preset_len,
                             bool *coolboost_enabled,
                             daemon_runtime_state *runtime, const char *cmd)
{
    if (handle_coolboost_command(client, ec, cfg, states, *auto_mode, preset,
                                 coolboost_enabled, runtime, cmd))
        return true;
    if (handle_fan_mode_command(client, ec, cfg, states, auto_mode, preset,
                                preset_len, coolboost_enabled, runtime, cmd))
        return true;
    if (handle_profile_command(client, ec, cfg, cmd))
        return true;
    if (handle_power_source_command(client, ec, cfg, states, *auto_mode, preset,
                                    *coolboost_enabled, runtime, cmd))
        return true;
    if (handle_keyboard_backlight_command(client, ec, cfg, states, *auto_mode,
                                          preset, *coolboost_enabled, runtime,
                                          cmd))
        return true;
    return handle_gpu_temp_command(client, cmd);
}
