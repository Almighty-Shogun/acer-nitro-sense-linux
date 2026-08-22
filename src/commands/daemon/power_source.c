#include "commands/daemon/platform_handlers.h"

#include "daemon/state.h"
#include "control/protocol.h"
#include "platform/power_source.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Handles one mutating power-source policy subcommand.
 *
 * Apply and auto both need the current power source and runtime persistence,
 * but status is handled earlier as a read-only registry action.
 */
typedef bool (*power_source_command_handler)(
    int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
);

/**
 * Maps a power-source subcommand name to its daemon handler.
 *
 * The table keeps mutating policy actions separate from power-source status
 * reporting.
 */
typedef struct
{
    const char* name;
    power_source_command_handler handler;
} power_source_command;

/**
 * Describes one power-source auto-apply toggle value.
 *
 * The command protocol uses on/off text while runtime state stores the
 * selected policy as a boolean.
 */
typedef struct
{
    const char* name;
    bool enabled;
} power_source_auto_value;

/**
 * Supported power-source auto-apply toggle values.
 *
 * The command protocol exposes on/off strings while daemon runtime state
 * stores the selected policy as a boolean.
 */
static const power_source_auto_value POWER_SOURCE_AUTO_VALUES[] = {
    {.name = "on", .enabled = true},
    {.name = "off", .enabled = false},
};

/**
 * Find the auto-apply toggle matching a parsed command token.
 *
 * Returning NULL keeps invalid auto values on the normal usage path.
 */
static const power_source_auto_value* find_power_source_auto_value(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(POWER_SOURCE_AUTO_VALUES); i++)
    {
        if (strcmp(name, POWER_SOURCE_AUTO_VALUES[i].name) == 0)
            return &POWER_SOURCE_AUTO_VALUES[i];
    }

    return NULL;
}

/**
 * Toggle automatic profile changes for AC and battery.
 *
 * Enabling auto-apply immediately applies the profile for the current power
 * source, then persists the policy for future AC or battery changes.
 */
static bool handle_power_source_auto_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const bool coolboost_enabled,
    daemon_runtime_state* runtime, /* NOLINT(readability-non-const-parameter): shared command handler signature. */
    const char* cmd
)
{
    char auto_value[16];

    const enum power_source_state source = read_power_source();
    const char* target_profile = power_source_profile_for(cfg, source);

    if (!parse_power_source_auto_command(cmd, auto_value, sizeof(auto_value)))
    {
        control_reply(client, "error usage: power-source status|apply|auto on|off\n");

        return true;
    }

    if (!power_source_profile_policy_available(cfg))
    {
        control_reply(client, "error power-source profile policy unavailable source=%s\n", power_source_name(source));

        return true;
    }

    const power_source_auto_value* entry = find_power_source_auto_value(auto_value);

    if (!entry)
    {
        control_reply(client, "error usage: power-source status|apply|auto on|off\n");

        return true;
    }

    runtime->power_source_auto_apply = entry->enabled;

    if (entry->enabled)
    {
        if (target_profile && !apply_power_source_profile(ec, cfg, source))
        {
            control_reply(client, "error power-source profile write failed\n");

            return true;
        }
    }

    write_control_state(cfg, states, auto_mode, preset, coolboost_enabled, runtime);

    if (!daemon_quiet_logs)
        fprintf(
            stderr,
            "power_source_auto_apply enabled=%d source=%s profile=%s\n",
            runtime->power_source_auto_apply ? 1 : 0,
            power_source_name(source),
            target_profile ? target_profile : "unavailable"
        );

    control_reply(
        client,
        "power_source=%s auto_apply=%s profile=%s\n",
        power_source_name(source),
        runtime->power_source_auto_apply ? "on" : "off",
        target_profile ? target_profile : "unavailable"
    );

    return true;
}

/**
 * Apply the profile configured for the current power source.
 *
 * This is a one-shot action; it does not enable automatic switching for later
 * AC or battery transitions.
 */
static bool handle_power_source_apply_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const bool coolboost_enabled,
    daemon_runtime_state* runtime, /* NOLINT(readability-non-const-parameter): shared command handler signature. */
    const char* cmd
)
{
    static const char* const actions[] = {"apply"};
    char action[16];
    const enum power_source_state source = read_power_source();
    const char* target_profile = power_source_profile_for(cfg, source);

    (void)states;
    (void)auto_mode;
    (void)preset;
    (void)coolboost_enabled;
    (void)runtime;

    if (!command_parse_enum_action(cmd, "power-source", actions, COMMAND_ARRAY_LEN(actions), action, sizeof(action)))
    {
        control_reply(client, "error usage: power-source status|apply|auto on|off\n");

        return true;
    }

    if (!target_profile)
    {
        control_reply(client, "error power-source profile policy unavailable source=%s\n", power_source_name(source));

        return true;
    }

    if (!apply_power_source_profile(ec, cfg, source))
    {
        control_reply(client, "error power-source profile write failed\n");

        return true;
    }

    if (!daemon_quiet_logs)
        fprintf(stderr, "power_source_profile_apply source=%s profile=%s\n", power_source_name(source), target_profile);

    control_reply(client, "power_source=%s profile=%s\n", power_source_name(source), target_profile);

    return true;
}

/**
 * Supported mutating power-source commands.
 *
 * Status is answered by the daemon registry, leaving only apply and auto
 * policy changes in this table.
 */
static const power_source_command POWER_SOURCE_COMMANDS[] = {
    {.name = "apply", .handler = handle_power_source_apply_command},
    {.name = "auto", .handler = handle_power_source_auto_command},
};

/**
 * Find a power-source subcommand entry.
 *
 * The status action is handled by the daemon registry before permission
 * checks; mutating actions route through this local table.
 */
static const power_source_command* find_power_source_command(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(POWER_SOURCE_COMMANDS); i++)
    {
        if (strcmp(name, POWER_SOURCE_COMMANDS[i].name) == 0)
            return &POWER_SOURCE_COMMANDS[i];
    }

    return NULL;
}

/**
 * Dispatch a power-source policy command.
 *
 * The dispatcher keeps status handling out of the mutating command path and
 * reports malformed subcommands with the shared power-source usage string.
 */
bool handle_power_source_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const bool coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd
)
{
    char action[16];

    if (!command_second_token(cmd, action, sizeof(action)))
    {
        control_reply(client, "error usage: power-source status|apply|auto on|off\n");

        return true;
    }

    const power_source_command* entry = find_power_source_command(action);

    if (!entry)
    {
        control_reply(client, "error usage: power-source status|apply|auto on|off\n");

        return true;
    }

    return entry->handler(client, ec, cfg, states, auto_mode, preset, coolboost_enabled, runtime, cmd);
}
