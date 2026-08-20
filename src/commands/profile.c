#include "commands/platform_handlers.h"

#include "commands/parser.h"
#include "platform/control.h"

#include <stdio.h>
#include <unistd.h>

bool handle_profile_command(const int client, struct ec_device *ec,
                            const struct ans_config *cfg, const char *cmd)
{
    char action[32];
    const struct platform_profile_entry *entry;

    if (!command_name_is(cmd, "profile"))
        return false;

    if (!parse_profile_command(cmd, action, sizeof(action))) {
        dprintf(client, "error usage: profile status|NAME\n");
        return true;
    }

    if (!cfg->platform_profiles.available) {
        dprintf(client, "error profiles unavailable for this model\n");
        return true;
    }

    entry = config_find_platform_profile(cfg, action);
    if (!entry) {
        dprintf(client, "error unknown profile\n");
        return true;
    }

    if (!apply_platform_profile(ec, cfg, action)) {
        dprintf(client, "error profile write failed\n");
        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(stderr, "profile_change profile=%s register=0x%02x value=0x%02x\n",
                action, cfg->platform_profiles.reg, entry->value);
    dprintf(client, "profile=%s value=0x%02x\n", action, entry->value);
    return true;
}
