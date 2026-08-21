#include "commands/client/platform.h"

#include "client/status.h"
#include "client/transport.h"
#include "client/usage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int client_handle_coolboost_command(const int argc, char **argv)
{
    if (argc != 3) {
        client_usage(stderr);
        return 2;
    }

    if (strcmp(argv[2], "on") != 0 && strcmp(argv[2], "off") != 0 &&
        strcmp(argv[2], "status") != 0) {
        fprintf(stderr, "coolboost must be on, off, or status\n");

        return 2;
    }

    if (strcmp(argv[2], "status") == 0)
        return client_print_coolboost_status();

    return client_send_commandf(false, "coolboost %s\n", argv[2]);
}

int client_handle_fan_mode_command(const int argc, char **argv)
{
    if (argc != 3) {
        client_usage(stderr);
        return 2;
    }

    if (strcmp(argv[2], "status") != 0 && strcmp(argv[2], "auto") != 0 &&
        strcmp(argv[2], "manual") != 0 && strcmp(argv[2], "turbo") != 0) {
        fprintf(stderr, "fan-mode must be status, auto, manual, or turbo\n");

        return 2;
    }

    return client_send_commandf(false, "fan-mode %s\n", argv[2]);
}

int client_handle_profile_command(const int argc, char **argv)
{
    if (argc != 3) {
        client_usage(stderr);
        return 2;
    }

    return client_send_commandf(false, "profile %s\n", argv[2]);
}

int client_handle_gpu_temp_command(const int argc, char **argv)
{
    if (argc != 3) {
        client_usage(stderr);
        return 2;
    }

    if (strcmp(argv[2], "status") != 0 && strcmp(argv[2], "auto") != 0 &&
        strcmp(argv[2], "live") != 0) {
        fprintf(stderr, "gpu-temp must be status, auto, or live\n");

        return 2;
    }

    return client_send_commandf(false, "gpu-temp %s\n", argv[2]);
}

int client_handle_power_source_command(const int argc, char **argv)
{
    if (argc != 3 && argc != 4) {
        client_usage(stderr);
        return 2;
    }

    if (argc == 4 && strcmp(argv[2], "auto") == 0) {
        if (strcmp(argv[3], "on") != 0 && strcmp(argv[3], "off") != 0) {
            fprintf(stderr, "power-source auto must be on or off\n");

            return 2;
        }

        return client_send_commandf(false, "power-source auto %s\n", argv[3]);
    }

    if (argc != 3 ||
        (strcmp(argv[2], "status") != 0 && strcmp(argv[2], "apply") != 0)) {
        fprintf(stderr, "power-source must be status, apply, auto on, or auto off\n");

        return 2;
    }

    return client_send_commandf(false, "power-source %s\n", argv[2]);
}

int client_handle_keyboard_backlight_command(const int argc, char **argv)
{
    char *end;
    long percent;

    if (argc != 3 && argc != 4) {
        client_usage(stderr);
        return 2;
    }

    if (argc == 3 && strcmp(argv[2], "status") == 0)
        return client_send_command("keyboard-backlight status\n", false);

    if (argc == 4 && strcmp(argv[2], "timeout") == 0) {
        if (strcmp(argv[3], "status") != 0 && strcmp(argv[3], "on") != 0 &&
            strcmp(argv[3], "off") != 0) {
            fprintf(stderr, "keyboard-backlight timeout must be status, on, or off\n");
            return 2;
        }

        return client_send_commandf(false, "keyboard-backlight timeout %s\n", argv[3]);
    }

    if (argc != 4 || strcmp(argv[2], "set") != 0) {
        fprintf(stderr,
                "keyboard-backlight must be status, set 0-100, or timeout status|on|off\n");
        return 2;
    }

    percent = strtol(argv[3], &end, 10);
    if (end == argv[3] || *end != '\0' || percent < 0 || percent > 100) {
        fprintf(stderr, "keyboard-backlight set must be 0-100\n");
        return 2;
    }

    return client_send_commandf(false, "keyboard-backlight set %ld\n", percent);
}
