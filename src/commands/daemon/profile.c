#include "commands/daemon/platform_handlers.h"

#include "config/config.h"
#include "control/protocol.h"
#include "platform/control.h"
#include "commands/parser/parser.h"

#include <stdio.h>

/**
 * Apply a firmware platform profile.
 *
 * The registry answers profile status before this function is reached, so the
 * handler only validates and writes named model profile entries.
 */
bool handle_profile_command(const int client, struct ec_device* ec, const struct ans_config* cfg, const char* cmd)
{
    char action[32];

    if (!parse_profile_command(cmd, action, sizeof(action)))
    {
        control_reply(client, "error usage: profile status|NAME\n");

        return true;
    }

    if (!cfg->platform_profiles.available)
    {
        control_reply(client, "error profiles unavailable for this model\n");

        return true;
    }

    const struct platform_profile_entry* entry = config_find_platform_profile(cfg, action);

    if (!entry)
    {
        control_reply(client, "error unknown profile\n");

        return true;
    }

    if (!apply_platform_profile(ec, cfg, action))
    {
        control_reply(client, "error profile write failed\n");

        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(
            stderr,
            "profile_change profile=%s register=0x%02x value=0x%02x\n",
            action,
            cfg->platform_profiles.reg,
            entry->value
        );

    control_reply(client, "profile=%s value=0x%02x\n", action, entry->value);

    return true;
}
