#include "commands/client/platform.h"

#include "client/usage.h"
#include "client/status.h"
#include "client/transport.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Handles one client-side platform subcommand.
 *
 * Platform commands share a command group but each action has its own argument
 * rules before a request is sent to the daemon.
 */
typedef int (*client_platform_subcommand_handler)(int argc, char** argv);

/**
 * Maps a platform subcommand name to its CLI handler.
 *
 * The table-driven form keeps user-facing subcommand names next to the code
 * that validates and forwards them.
 */
typedef struct
{
    const char* name;
    client_platform_subcommand_handler handler;
} client_platform_subcommand;

/**
 * Find a client-side platform subcommand entry.
 *
 * Command groups reuse this lookup so all platform commands fail in the same
 * predictable way when a subcommand is unknown.
 */
static const client_platform_subcommand* find_client_platform_subcommand(
    const client_platform_subcommand* commands,
    const size_t command_len,
    const char* name
)
{
    for (size_t i = 0; i < command_len; i++)
    {
        if (strcmp(name, commands[i].name) == 0)
            return &commands[i];
    }

    return NULL;
}

/**
 * Forward coolboost on/off requests to the daemon.
 *
 * The client accepts only the already-routed on/off action here; capability
 * checks and EC writes remain owned by the daemon.
 */
static int client_handle_coolboost_set_command(const int argc, char** argv)
{
    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    return client_send_commandf(false, "coolboost %s\n", argv[2]);
}

/**
 * Print the current daemon coolboost status.
 *
 * Status output is handled by the richer client status helper because it has
 * daemon response parsing and formatting in one place.
 */
static int client_handle_coolboost_status_command(const int argc, char** argv)
{
    (void)argv;

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    return client_print_coolboost_status();
}

/**
 * Dispatch CoolBoost client actions.
 *
 * The client validates the action token before choosing between local status
 * formatting and daemon-backed on/off requests.
 */
int client_handle_coolboost_command(const int argc, char** argv)
{
    static const client_platform_subcommand commands[] = {
        {.name = "on", .handler = client_handle_coolboost_set_command},
        {.name = "off", .handler = client_handle_coolboost_set_command},
        {.name = "status", .handler = client_handle_coolboost_status_command},
    };

    if (argc < 3)
    {
        client_usage(stderr);

        return 2;
    }

    const client_platform_subcommand* command =
        find_client_platform_subcommand(commands, COMMAND_ARRAY_LEN(commands), argv[2]);

    if (!command)
    {
        fprintf(stderr, "coolboost must be on, off, or status\n");

        return 2;
    }

    return command->handler(argc, argv);
}

/**
 * Forward a firmware fan-mode action.
 *
 * Fan-mode actions are a closed set because the daemon maps them directly to
 * model-specific firmware values.
 */
int client_handle_fan_mode_command(const int argc, char** argv)
{
    static const char* const actions[] = {"status", "auto", "manual", "turbo"};

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    if (!command_token_in(argv[2], actions, COMMAND_ARRAY_LEN(actions)))
    {
        fprintf(stderr, "fan-mode must be status, auto, manual, or turbo\n");

        return 2;
    }

    return client_send_commandf(false, "fan-mode %s\n", argv[2]);
}

/**
 * Forward a platform profile action.
 *
 * Profile names are model-defined, so the client only validates command shape
 * and leaves availability checks to the daemon.
 */
int client_handle_profile_command(const int argc, char** argv)
{
    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    return client_send_commandf(false, "profile %s\n", argv[2]);
}

/**
 * Forward a GPU temperature policy action.
 *
 * The client accepts only the public policy names and lets the daemon apply the
 * matching Linux runtime-power setting.
 */
int client_handle_gpu_temp_command(const int argc, char** argv)
{
    static const char* const actions[] = {"status", "auto", "live"};

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    if (!command_token_in(argv[2], actions, COMMAND_ARRAY_LEN(actions)))
    {
        fprintf(stderr, "gpu-temp must be status, auto, or live\n");

        return 2;
    }

    return client_send_commandf(false, "gpu-temp %s\n", argv[2]);
}

/**
 * Request the current power-source policy from the daemon.
 *
 * This is read-only daemon state and therefore uses a plain socket request
 * after validating that no extra CLI arguments were supplied.
 */
static int client_handle_power_source_status_command(const int argc, char** argv)
{
    (void)argv;

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    return client_send_command("power-source status\n", false);
}

/**
 * Ask the daemon to apply the configured AC or battery profile now.
 *
 * The daemon owns source detection and firmware writes, so the client only
 * validates the action shape and forwards the request.
 */
static int client_handle_power_source_apply_command(const int argc, char** argv)
{
    (void)argv;

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    return client_send_command("power-source apply\n", false);
}

/**
 * Enable or disable automatic AC/battery profile switching.
 *
 * Only on/off values are accepted locally. The daemon persists the resulting
 * policy and applies the current source profile when enabling it.
 */
static int client_handle_power_source_auto_command(const int argc, char** argv)
{
    static const char* const auto_values[] = {"on", "off"};

    if (argc != 4)
    {
        client_usage(stderr);

        return 2;
    }

    if (!command_token_in(argv[3], auto_values, COMMAND_ARRAY_LEN(auto_values)))
    {
        fprintf(stderr, "power-source auto must be on or off\n");

        return 2;
    }

    return client_send_commandf(false, "power-source auto %s\n", argv[3]);
}

/**
 * Dispatch power-source profile actions.
 *
 * Status, immediate apply, and auto-toggle actions share the same command group
 * but have different argument counts.
 */
int client_handle_power_source_command(const int argc, char** argv)
{
    static const client_platform_subcommand commands[] = {
        {.name = "status", .handler = client_handle_power_source_status_command},
        {.name = "apply", .handler = client_handle_power_source_apply_command},
        {.name = "auto", .handler = client_handle_power_source_auto_command},
    };

    if (argc < 3)
    {
        client_usage(stderr);

        return 2;
    }

    const client_platform_subcommand* command =
        find_client_platform_subcommand(commands, COMMAND_ARRAY_LEN(commands), argv[2]);

    if (!command)
    {
        fprintf(stderr, "power-source must be status, apply, auto on, or auto off\n");

        return 2;
    }

    return command->handler(argc, argv);
}

/**
 * Request the current keyboard backlight status from the daemon.
 *
 * The daemon reports both EC-backed and unsupported states so the client does
 * not need to know which backend a model uses.
 */
static int client_handle_keyboard_backlight_status_command(const int argc, char** argv)
{
    (void)argv;

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    return client_send_command("keyboard-backlight status\n", false);
}

/**
 * Validate a supported brightness step before forwarding it to the daemon.
 *
 * Nitro keyboard brightness is stepped rather than continuous, so accepting
 * arbitrary percentages would imply precision the hardware does not expose.
 */
static int client_handle_keyboard_backlight_set_command(const int argc, char** argv)
{
    char* end;

    static const int brightness_steps[] = {0, 25, 50, 75, 100};

    if (argc != 4)
    {
        client_usage(stderr);

        return 2;
    }

    const long percent = strtol(argv[3], &end, 10);

    if (end == argv[3] || *end != '\0' || !command_int_in((int)percent, brightness_steps, COMMAND_ARRAY_LEN(brightness_steps)))
    {
        fprintf(stderr, "keyboard-backlight set must be 0, 25, 50, 75, or 100\n");

        return 2;
    }

    return client_send_commandf(false, "keyboard-backlight set %ld\n", percent);
}

/**
 * Forward keyboard backlight timeout status and toggle actions.
 *
 * The timeout feature is daemon-managed because it depends on input activity
 * tracking and persisted runtime state.
 */
static int client_handle_keyboard_backlight_timeout_command(const int argc, char** argv)
{
    static const char* const timeout_actions[] = {"status", "on", "off"};

    if (argc != 4)
    {
        client_usage(stderr);

        return 2;
    }

    if (!command_token_in(argv[3], timeout_actions, COMMAND_ARRAY_LEN(timeout_actions)))
    {
        fprintf(stderr, "keyboard-backlight timeout must be status, on, or off\n");

        return 2;
    }

    return client_send_commandf(false, "keyboard-backlight timeout %s\n", argv[3]);
}

/**
 * Dispatch keyboard backlight actions.
 *
 * Brightness and timeout actions are validated locally so unsupported command
 * shapes fail before reaching the daemon.
 */
int client_handle_keyboard_backlight_command(const int argc, char** argv)
{
    static const client_platform_subcommand commands[] = {
        {.name = "status", .handler = client_handle_keyboard_backlight_status_command},
        {.name = "set", .handler = client_handle_keyboard_backlight_set_command},
        {.name = "timeout", .handler = client_handle_keyboard_backlight_timeout_command},
    };

    if (argc < 3)
    {
        client_usage(stderr);

        return 2;
    }

    const client_platform_subcommand* command =
        find_client_platform_subcommand(commands, COMMAND_ARRAY_LEN(commands), argv[2]);

    if (!command)
    {
        fprintf(stderr, "keyboard-backlight must be status, set 0|25|50|75|100, or timeout status|on|off\n");

        return 2;
    }

    return command->handler(argc, argv);
}
