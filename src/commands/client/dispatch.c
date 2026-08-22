#include "commands/client/dispatch.h"

#include "commands/client/builtin.h"
#include "commands/client/fan.h"
#include "commands/client/platform.h"
#include "client/usage.h"

#include <stdio.h>
#include <string.h>

typedef int (*client_command_handler)(int argc, char **argv);

struct client_command {
    const char *name;
    client_command_handler handler;
};

static const struct client_command CLIENT_COMMANDS[] = {
    {"start", client_handle_start_command},
    {"restart", client_handle_restart_command},
    {"stop", client_handle_stop_command},
    {"status", client_handle_status_command},
    {"auto", client_handle_auto_command},
    {"firmware-auto", client_handle_firmware_auto_command},
    {"presets", client_handle_presets_command},
    {"capabilities", client_handle_capabilities_command},
    {"resume", client_handle_resume_command},
    {"validate", client_handle_validate_command},
    {"doctor", client_handle_doctor_command},
    {"coolboost", client_handle_coolboost_command},
    {"fan-mode", client_handle_fan_mode_command},
    {"profile", client_handle_profile_command},
    {"gpu-temp", client_handle_gpu_temp_command},
    {"power-source", client_handle_power_source_command},
    {"keyboard-backlight", client_handle_keyboard_backlight_command},
    {"ec", client_handle_ec_command},
    {"set", client_handle_set_command},
    {"preset", client_handle_preset_command},
};

int client_run(const int argc, char **argv)
{
    const size_t command_count = sizeof(CLIENT_COMMANDS) / sizeof(CLIENT_COMMANDS[0]);

    if (argc < 2) {
        client_usage(stderr);
        return 2;
    }

    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(argv[1], CLIENT_COMMANDS[i].name) == 0)
            return CLIENT_COMMANDS[i].handler(argc, argv);
    }

    client_usage(stderr);

    return 2;
}
