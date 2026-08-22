#include "commands/client/builtin.h"

#include "client/doctor.h"
#include "client/service.h"
#include "client/status.h"
#include "client/transport.h"
#include "client/usage.h"

#include <stdio.h>
#include <string.h>

int client_handle_status_command(const int argc, char **argv)
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

int client_handle_start_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_run_systemctl("start");
}

int client_handle_restart_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_run_systemctl("restart");
}

int client_handle_stop_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (client_send_command("stop\n", false) == 0)
        return 0;

    return client_run_systemctl("stop");
}

int client_handle_auto_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("auto\n", false);
}

int client_handle_firmware_auto_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("firmware-auto\n", false);
}

int client_handle_presets_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("presets\n", false);
}

int client_handle_capabilities_command(const int argc, char **argv)
{
    (void)argv;

    if (argc != 2) {
        client_usage(stderr);
        return 2;
    }

    return client_send_command("capabilities\n", false);
}

int client_handle_resume_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_send_command("resume\n", false);
}

int client_handle_validate_command(const int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return client_validate_model();
}

int client_handle_doctor_command(const int argc, char **argv)
{
    (void)argv;

    if (argc != 2) {
        client_usage(stderr);
        return 2;
    }

    return client_doctor();
}
