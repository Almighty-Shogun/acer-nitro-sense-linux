#include "commands/client/fan.h"

#include "client/parse.h"
#include "client/usage.h"
#include "client/transport.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Handles one client-side EC subcommand.
 *
 * EC debug commands are grouped under one user-facing command but have
 * different argument shapes for single-register reads and bounded dumps.
 */
typedef int (*client_ec_handler)(int argc, char** argv);

/**
 * Maps an EC subcommand name to its CLI handler.
 *
 * Keeping EC routing table-driven makes it clear which privileged debug
 * actions the client can request from the daemon.
 */
typedef struct
{
    const char* name;
    client_ec_handler handler;
} client_ec_command;

/**
 * Handles one client-side preset subcommand.
 *
 * Named preset application and preset status share the preset command group
 * even though only status is handled specially by the client.
 */
typedef int (*client_preset_handler)(int argc, char** argv);

/**
 * Maps a preset subcommand name to its CLI handler.
 *
 * Unknown preset arguments intentionally fall through to daemon preset
 * application, so only reserved subcommands are listed here.
 */
typedef struct
{
    const char* name;
    client_preset_handler handler;
} client_preset_command;

/**
 * Request a single EC register read.
 *
 * The client validates the register byte before forwarding the daemon's flat
 * `ec-read` protocol command.
 */
static int client_handle_ec_read_command(const int argc, char** argv)
{
    int start;

    if (argc != 4)
    {
        client_usage(stderr);

        return 2;
    }

    if (!client_parse_byte_value(argv[3], &start))
    {
        fprintf(stderr, "register must be 0-255\n");

        return 2;
    }

    return client_send_commandf(false, "ec-read %d\n", start);
}

/**
 * Request a bounded EC register dump.
 *
 * The client enforces byte values and a maximum range before asking the daemon
 * for raw EC diagnostics.
 */
static int client_handle_ec_dump_command(const int argc, char** argv)
{
    int start, end;

    if (argc != 5)
    {
        client_usage(stderr);

        return 2;
    }

    if (!client_parse_byte_value(argv[3], &start) || !client_parse_byte_value(argv[4], &end))
    {
        fprintf(stderr, "registers must be 0-255\n");

        return 2;
    }

    if (end < start || end - start > 127)
    {
        fprintf(stderr, "dump range must be ascending and at most 128 bytes\n");

        return 2;
    }

    return client_send_commandf(false, "ec-dump %d %d\n", start, end);
}

/**
 * Supported client EC debug subcommands.
 *
 * The client exposes read and dump as friendly subcommands while the daemon
 * receives the flat protocol names it already understands.
 */
static const client_ec_command CLIENT_EC_COMMANDS[] = {
    {.name = "read", .handler = client_handle_ec_read_command},
    {.name = "dump", .handler = client_handle_ec_dump_command},
};

/**
 * Find a client EC subcommand entry.
 *
 * CLI EC routing mirrors daemon EC routing so supported subcommands stay
 * visible in one table.
 */
static const client_ec_command* find_client_ec_command(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(CLIENT_EC_COMMANDS); i++)
    {
        if (strcmp(name, CLIENT_EC_COMMANDS[i].name) == 0)
            return &CLIENT_EC_COMMANDS[i];
    }

    return NULL;
}

/**
 * Dispatch a client EC debug command.
 *
 * `ec read` and `ec dump` are user-facing aliases for the daemon protocol
 * commands exposed through the control socket.
 */
int client_handle_ec_command(const int argc, char** argv)
{
    if (argc < 3)
    {
        client_usage(stderr);

        return 2;
    }

    const client_ec_command* entry = find_client_ec_command(argv[2]);

    if (entry)
        return entry->handler(argc, argv);

    client_usage(stderr);

    return 2;
}

/**
 * Set a manual fan percentage.
 *
 * The client validates the fan selector and percentage before forwarding the
 * manual fan-control request to the daemon.
 */
int client_handle_set_command(const int argc, char** argv)
{
    int percent;

    static const char* const fans[] = {"cpu", "gpu", "all"};

    if (argc != 4)
    {
        client_usage(stderr);

        return 2;
    }

    if (!client_parse_percent(argv[3], &percent))
    {
        fprintf(stderr, "percent must be 1-100\n");

        return 2;
    }

    if (!command_token_in(argv[2], fans, COMMAND_ARRAY_LEN(fans)))
    {
        fprintf(stderr, "fan must be cpu, gpu, or all\n");

        return 2;
    }

    return client_send_commandf(false, "set %s %d\n", argv[2], percent);
}

/**
 * Ask the daemon to report the active preset context.
 *
 * This avoids overloading normal preset application with a reserved preset
 * name while still keeping the user-facing command under `preset`.
 */
static int client_handle_preset_show_command(const int argc, char** argv)
{
    (void)argv;

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    return client_send_command("preset-show\n", false);
}

/**
 * Find a client preset subcommand entry.
 *
 * Returning NULL tells the caller that the token is a preset name rather than
 * a reserved local subcommand.
 */
static const client_preset_command* find_client_preset_command(
    const client_preset_command* commands,
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
 * Apply or inspect a fan preset.
 *
 * Reserved preset subcommands are handled locally; every other token is sent
 * to the daemon as a model-defined preset name.
 */
int client_handle_preset_command(const int argc, char** argv)
{
    static const client_preset_command commands[] = {
        {.name = "show", .handler = client_handle_preset_show_command},
    };

    if (argc != 3)
    {
        client_usage(stderr);

        return 2;
    }

    const client_preset_command* command = find_client_preset_command(commands, COMMAND_ARRAY_LEN(commands), argv[2]);

    if (command)
        return command->handler(argc, argv);

    return client_send_commandf(false, "preset %s\n", argv[2]);
}
