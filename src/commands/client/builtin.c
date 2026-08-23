#include "commands/client/builtin.h"

#include "client/usage.h"
#include "client/doctor.h"
#include "client/status.h"
#include "client/service.h"
#include "client/transport.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Describes one supported status output flag.
 *
 * Status accepts a small set of optional flags that affect presentation only.
 * Keeping the parsed behavior in a table avoids another chain of string
 * comparisons in the command handler.
 */
typedef struct
{
    const char* name;
    enum temp_unit unit;
    bool json;
} status_option;

/**
 * Supported one-argument variants for the status command.
 *
 * Each entry maps the literal CLI token to the status formatter settings it
 * enables.
 */
static const status_option STATUS_OPTIONS[] = {
    {.name = "--json", .unit = TEMP_UNIT_CELSIUS, .json = true},
    {.name = "--celsius", .unit = TEMP_UNIT_CELSIUS, .json = false},
    {.name = "--fahrenheit", .unit = TEMP_UNIT_FAHRENHEIT, .json = false},
};

/**
 * Find the status option matching a CLI flag.
 *
 * Unknown flags return NULL so the caller can print the normal command usage
 * instead of leaking parser details into user-facing output.
 */
static const status_option* find_status_option(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(STATUS_OPTIONS); i++)
    {
        if (strcmp(name, STATUS_OPTIONS[i].name) == 0)
            return &STATUS_OPTIONS[i];
    }

    return NULL;
}

/**
 * Require an exact argument count for commands without subcommands.
 *
 * Simple passthrough commands still need strict validation so accidental extra
 * arguments are rejected before a daemon or systemctl command is executed.
 */
static bool client_expect_argc(const int argc, const int expected)
{
    if (argc == expected)
        return true;

    client_usage(stderr);

    return false;
}

/**
 * Print daemon status.
 *
 * Status is the only builtin command with local presentation flags. The
 * parsed unit and JSON options are passed to the status formatter directly.
 */
int client_handle_status_command(const int argc, char** argv)
{
    enum temp_unit unit = TEMP_UNIT_CELSIUS;

    bool json = false;

    if (argc > 3)
    {
        client_usage(stderr);

        return 2;
    }

    if (argc == 3)
    {
        const status_option* option = find_status_option(argv[2]);

        if (!option)
        {
            client_usage(stderr);

            return 2;
        }

        unit = option->unit;
        json = option->json;
    }

    return client_print_status(unit, json);
}

/**
 * Start the systemd daemon service.
 *
 * The command rejects extra arguments before delegating to systemd so service
 * lifecycle commands stay predictable.
 */
int client_handle_start_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    return client_run_systemctl("start");
}

/**
 * Restart the systemd daemon service.
 *
 * Restart is intentionally systemd-backed because it needs to replace the
 * long-running daemon process rather than only update daemon state.
 */
int client_handle_restart_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    return client_run_systemctl("restart");
}

/**
 * Stop the daemon service.
 *
 * A live daemon gets the chance to shut down through the control socket before
 * the client falls back to systemd.
 */
int client_handle_stop_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    if (client_send_command("stop\n", false) == 0)
        return 0;

    return client_run_systemctl("stop");
}

/**
 * Enable the daemon automatic fan curve.
 *
 * This is a direct control-socket command after validating that no extra CLI
 * arguments were supplied.
 */
int client_handle_auto_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    return client_send_command("auto\n", false);
}

/**
 * Enable firmware automatic fan control.
 *
 * Firmware-auto is forwarded to the daemon so it can write model-specific
 * fan-mode registers and persist the resulting state.
 */
int client_handle_firmware_auto_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    return client_send_command("firmware-auto\n", false);
}

/**
 * List configured fan presets.
 *
 * Presets are read-only model metadata, so the client simply forwards the
 * command after strict argument validation.
 */
int client_handle_presets_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    return client_send_command("presets\n", false);
}

/**
 * Print feature capabilities.
 *
 * Capability output describes fan, platform, sensor, and keyboard support for
 * the active model profile.
 */
int client_handle_capabilities_command(const int argc, char** argv)
{
    (void)argv;

    if (argc != 2)
    {
        client_usage(stderr);

        return 2;
    }

    return client_send_command("capabilities\n", false);
}

/**
 * Reapply daemon state after resume.
 *
 * Resume is forwarded to the daemon because EC reinitialization depends on the
 * current model and persisted control state.
 */
int client_handle_resume_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    return client_send_command("resume\n", false);
}

/**
 * Validate the active model configuration.
 *
 * Validation runs locally through the client helper and does not require a
 * long-running daemon socket.
 */
int client_handle_validate_command(const int argc, char** argv)
{
    (void)argv;

    if (!client_expect_argc(argc, 2))
        return 2;

    return client_validate_model();
}

/**
 * Print diagnostic information.
 *
 * Doctor output gathers install, permission, sensor, and platform state for
 * support and model bring-up.
 */
int client_handle_doctor_command(const int argc, char** argv)
{
    (void)argv;

    if (argc != 2)
    {
        client_usage(stderr);

        return 2;
    }

    return client_doctor();
}
