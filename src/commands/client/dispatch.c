#include "commands/client/dispatch.h"

#include "client/doctor.h"
#include "commands/client/fan.h"
#include "commands/client/platform.h"
#include "client/service.h"
#include "client/status.h"
#include "client/transport.h"
#include "client/usage.h"

#include <stdio.h>
#include <string.h>

typedef int (*client_command_handler)(int argc, char **argv);

struct client_command {
    const char *name;
    client_command_handler handler;
};

static int handle_status_command(const int argc, char **argv)
{
    enum temp_unit unit = TEMP_UNIT_CELSIUS;
    bool json = false;

    if (argc > 3) {
        client_usage(stderr);
        return 2;
    }

    if (argc == 3) {
        if (strcmp(argv[2], "--json") == 0)
            json = true;
        else if (strcmp(argv[2], "--celsius") == 0)
            unit = TEMP_UNIT_CELSIUS;
        else if (strcmp(argv[2], "--fahrenheit") == 0)
            unit = TEMP_UNIT_FAHRENHEIT;
        else {
            client_usage(stderr);
            return 2;
        }
    }

    return client_print_status(unit, json);
}

static int handle_start_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_run_systemctl("start");
}

static int handle_restart_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_run_systemctl("restart");
}

static int handle_stop_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (client_send_command("stop\n", false) == 0)
        return 0;

    return client_run_systemctl("stop");
}

static int handle_auto_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("auto\n", false);
}

static int handle_firmware_auto_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("firmware-auto\n", false);
}

static int handle_presets_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("presets\n", false);
}

static int handle_capabilities_command(const int argc, char **argv)
{
    (void)argv;

    if (argc != 2) {
        client_usage(stderr);
        return 2;
    }

    return client_send_command("capabilities\n", false);
}

static int handle_resume_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("resume\n", false);
}

static int handle_validate_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_validate_model();
}

static int handle_doctor_command(const int argc, char **argv)
{
    (void)argv;

    if (argc != 2) {
        client_usage(stderr);
        return 2;
    }

    return client_doctor();
}

static const struct client_command CLIENT_COMMANDS[] = {
    {"start", handle_start_command},
    {"restart", handle_restart_command},
    {"stop", handle_stop_command},
    {"status", handle_status_command},
    {"auto", handle_auto_command},
    {"firmware-auto", handle_firmware_auto_command},
    {"presets", handle_presets_command},
    {"capabilities", handle_capabilities_command},
    {"resume", handle_resume_command},
    {"validate", handle_validate_command},
    {"doctor", handle_doctor_command},
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
