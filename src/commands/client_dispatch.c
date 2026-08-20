#include "commands/client_dispatch.h"

#include "client/doctor.h"
#include "commands/client_fan.h"
#include "commands/client_platform.h"
#include "client/service.h"
#include "client/status.h"
#include "client/transport.h"
#include "client/usage.h"

#include <stdio.h>
#include <string.h>

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

int client_run(const int argc, char **argv)
{
    if (argc < 2) {
        client_usage(stderr);
        return 2;
    }

    if (strcmp(argv[1], "start") == 0)
        return client_run_systemctl("start");

    if (strcmp(argv[1], "restart") == 0)
        return client_run_systemctl("restart");

    if (strcmp(argv[1], "stop") == 0) {
        if (client_send_command("stop\n", false) == 0)
            return 0;

        return client_run_systemctl("stop");
    }

    if (strcmp(argv[1], "status") == 0)
        return handle_status_command(argc, argv);

    if (strcmp(argv[1], "auto") == 0)
        return client_send_command("auto\n", false);

    if (strcmp(argv[1], "firmware-auto") == 0)
        return client_send_command("firmware-auto\n", false);

    if (strcmp(argv[1], "presets") == 0)
        return client_send_command("presets\n", false);

    if (strcmp(argv[1], "capabilities") == 0 && argc == 2)
        return client_send_command("capabilities\n", false);

    if (strcmp(argv[1], "resume") == 0)
        return client_send_command("resume\n", false);

    if (strcmp(argv[1], "validate") == 0)
        return client_validate_model();

    if (strcmp(argv[1], "doctor") == 0 && argc == 2)
        return client_doctor();

    if (strcmp(argv[1], "coolboost") == 0)
        return client_handle_coolboost_command(argc, argv);

    if (strcmp(argv[1], "fan-mode") == 0)
        return client_handle_fan_mode_command(argc, argv);

    if (strcmp(argv[1], "profile") == 0)
        return client_handle_profile_command(argc, argv);

    if (strcmp(argv[1], "gpu-temp") == 0)
        return client_handle_gpu_temp_command(argc, argv);

    if (strcmp(argv[1], "power-source") == 0)
        return client_handle_power_source_command(argc, argv);

    if (strcmp(argv[1], "keyboard-backlight") == 0)
        return client_handle_keyboard_backlight_command(argc, argv);

    if (strcmp(argv[1], "ec") == 0)
        return client_handle_ec_command(argc, argv);

    if (strcmp(argv[1], "set") == 0)
        return client_handle_set_command(argc, argv);

    if (strcmp(argv[1], "preset") == 0)
        return client_handle_preset_command(argc, argv);

    client_usage(stderr);

    return 2;
}
