#include "commands/client/dispatch.h"

#include "client/usage.h"
#include "commands/client/fan.h"
#include "commands/client/builtin.h"
#include "commands/client/platform.h"

#include <stdio.h>
#include <string.h>

/**
 * Supported top-level client commands.
 *
 * The client keeps user-facing command routing in one table so daemon
 * transport and command-specific validation stay separate.
 */
static const client_command CLIENT_COMMANDS[] = {
    {.name = "start", .handler = client_handle_start_command},
    {.name = "restart", .handler = client_handle_restart_command},
    {.name = "stop", .handler = client_handle_stop_command},
    {.name = "status", .handler = client_handle_status_command},
    {.name = "auto", .handler = client_handle_auto_command},
    {.name = "firmware-auto", .handler = client_handle_firmware_auto_command},
    {.name = "presets", .handler = client_handle_presets_command},
    {.name = "capabilities", .handler = client_handle_capabilities_command},
    {.name = "resume", .handler = client_handle_resume_command},
    {.name = "validate", .handler = client_handle_validate_command},
    {.name = "doctor", .handler = client_handle_doctor_command},
    {.name = "coolboost", .handler = client_handle_coolboost_command},
    {.name = "fan-mode", .handler = client_handle_fan_mode_command},
    {.name = "profile", .handler = client_handle_profile_command},
    {.name = "gpu-temp", .handler = client_handle_gpu_temp_command},
    {.name = "power-source", .handler = client_handle_power_source_command},
    {.name = "keyboard-backlight", .handler = client_handle_keyboard_backlight_command},
    {.name = "ec", .handler = client_handle_ec_command},
    {.name = "set", .handler = client_handle_set_command},
    {.name = "preset", .handler = client_handle_preset_command},
};

/**
 * Dispatch the requested client command.
 *
 * Unknown commands and missing command names both fall back to the shared
 * usage output so every invalid invocation exits the same way.
 */
int client_run(const int argc, char** argv)
{
    const size_t command_count = sizeof(CLIENT_COMMANDS) / sizeof(CLIENT_COMMANDS[0]);

    if (argc < 2)
    {
        client_usage(stderr);

        return 2;
    }

    for (size_t i = 0; i < command_count; i++)
    {
        if (strcmp(argv[1], CLIENT_COMMANDS[i].name) == 0)
            return CLIENT_COMMANDS[i].handler(argc, argv);
    }

    client_usage(stderr);

    return 2;
}
