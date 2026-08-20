#include "commands/platform_handlers.h"

#include "commands/parser.h"
#include "daemon/state.h"
#include "platform/control.h"
#include "platform/power_source.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

bool handle_power_source_command(const int client, struct ec_device *ec,
                                 const struct ans_config *cfg,
                                 const fan_state states[ANS_MAX_FANS],
                                 const bool auto_mode, const char *preset,
                                 const bool coolboost_enabled,
                                 daemon_runtime_state *runtime,
                                 const char *cmd)
{
    char action[16];
    char auto_value[16];
    const enum power_source_state source = read_power_source();
    const char *target_profile = power_source_profile_for(cfg, source);

    if (!command_name_is(cmd, "power-source"))
        return false;

    if (parse_power_source_auto_command(cmd, auto_value, sizeof(auto_value))) {
        if (!power_source_profile_policy_available(cfg)) {
            dprintf(client, "error power-source profile policy unavailable source=%s\n",
                    power_source_name(source));
            return true;
        }

        if (strcmp(auto_value, "on") == 0) {
            runtime->power_source_auto_apply = true;
            if (target_profile && !apply_power_source_profile(ec, cfg, source)) {
                dprintf(client, "error power-source profile write failed\n");
                return true;
            }
        } else if (strcmp(auto_value, "off") == 0) {
            runtime->power_source_auto_apply = false;
        } else {
            dprintf(client, "error usage: power-source status|apply|auto on|off\n");
            return true;
        }

        write_control_state(cfg, states, auto_mode, preset, coolboost_enabled,
                            runtime);
        if (!daemon_quiet_logs)
            fprintf(stderr, "power_source_auto_apply enabled=%d source=%s profile=%s\n",
                    runtime->power_source_auto_apply ? 1 : 0,
                    power_source_name(source),
                    target_profile ? target_profile : "unavailable");
        dprintf(client, "power_source=%s auto_apply=%s profile=%s\n",
                power_source_name(source),
                runtime->power_source_auto_apply ? "on" : "off",
                target_profile ? target_profile : "unavailable");
        return true;
    }

    if (!parse_power_source_command(cmd, action, sizeof(action)) ||
        strcmp(action, "apply") != 0) {
        dprintf(client, "error usage: power-source status|apply|auto on|off\n");
        return true;
    }

    if (!target_profile) {
        dprintf(client, "error power-source profile policy unavailable source=%s\n",
                power_source_name(source));
        return true;
    }

    if (!apply_power_source_profile(ec, cfg, source)) {
        dprintf(client, "error power-source profile write failed\n");
        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(stderr, "power_source_profile_apply source=%s profile=%s\n",
                power_source_name(source), target_profile);
    dprintf(client, "power_source=%s profile=%s\n",
            power_source_name(source), target_profile);
    return true;
}
