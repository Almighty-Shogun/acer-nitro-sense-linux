#include "commands/readonly.h"

#include "commands/parser.h"
#include "daemon/capabilities.h"
#include "daemon/feature_status.h"
#include "daemon/status.h"

#include <string.h>

bool handle_readonly_command(const int client, struct ec_device *ec,
                             const struct ans_config *cfg,
                             const fan_state states[ANS_MAX_FANS],
                             const bool auto_mode, const char *preset,
                             const bool coolboost_enabled,
                             const daemon_runtime_state *runtime,
                             const char *cmd)
{
    char action[32];

    if (command_is_exact(cmd, "status")) {
        reply_status(client, cfg, states, auto_mode, preset, coolboost_enabled);
        return true;
    }

    if (command_is_exact(cmd, "presets")) {
        reply_presets(client, cfg);
        return true;
    }

    if (command_is_exact(cmd, "preset-show")) {
        reply_preset_show(client, cfg, states, auto_mode, preset);
        return true;
    }

    if (command_is_exact(cmd, "capabilities")) {
        reply_capabilities(client, cfg, runtime);
        return true;
    }

    if (parse_coolboost_command(cmd, action, sizeof(action)) &&
        strcmp(action, "status") == 0) {
        reply_coolboost_status(client, cfg, coolboost_enabled);
        return true;
    }

    if (parse_fan_mode_command(cmd, action, sizeof(action)) &&
        strcmp(action, "status") == 0) {
        reply_fan_mode_status(client, ec, cfg);
        return true;
    }

    if (parse_profile_command(cmd, action, sizeof(action)) &&
        strcmp(action, "status") == 0) {
        reply_profile_status(client, ec, cfg);
        return true;
    }

    if (parse_power_source_command(cmd, action, sizeof(action)) &&
        strcmp(action, "status") == 0) {
        reply_power_source_status(client, ec, cfg, runtime);
        return true;
    }

    if (parse_gpu_temp_command(cmd, action, sizeof(action)) &&
        strcmp(action, "status") == 0) {
        reply_gpu_temp_status(client, ec, cfg);
        return true;
    }

    if (parse_keyboard_backlight_command(cmd, action, sizeof(action)) &&
        strcmp(action, "status") == 0) {
        reply_keyboard_backlight_status(client, ec, cfg, runtime);
        return true;
    }

    if (parse_keyboard_backlight_timeout_command(cmd, action, sizeof(action)) &&
        strcmp(action, "status") == 0) {
        reply_keyboard_backlight_status(client, ec, cfg, runtime);
        return true;
    }

    return false;
}
