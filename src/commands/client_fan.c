#include "commands/client_fan.h"

#include "client/parse.h"
#include "client/transport.h"
#include "client/usage.h"

#include <stdio.h>
#include <string.h>

int client_handle_ec_command(const int argc, char **argv)
{
    char cmd[128];
    int start;
    int end;

    if (argc < 4) {
        client_usage(stderr);
        return 2;
    }

    if (strcmp(argv[2], "read") == 0 && argc == 4) {
        if (!client_parse_byte_value(argv[3], &start)) {
            fprintf(stderr, "register must be 0-255\n");

            return 2;
        }

        snprintf(cmd, sizeof(cmd), "ec-read %d\n", start);

        return client_send_command(cmd, false);
    }

    if (strcmp(argv[2], "dump") == 0 && argc == 5) {
        if (!client_parse_byte_value(argv[3], &start) ||
            !client_parse_byte_value(argv[4], &end)) {
            fprintf(stderr, "registers must be 0-255\n");

            return 2;
        }
        if (end < start || end - start > 127) {
            fprintf(stderr, "dump range must be ascending and at most 128 bytes\n");

            return 2;
        }

        snprintf(cmd, sizeof(cmd), "ec-dump %d %d\n", start, end);

        return client_send_command(cmd, false);
    }

    client_usage(stderr);

    return 2;
}

int client_handle_set_command(const int argc, char **argv)
{
    char cmd[128];
    int percent;

    if (argc != 4) {
        client_usage(stderr);
        return 2;
    }

    if (!client_parse_percent(argv[3], &percent)) {
        fprintf(stderr, "percent must be 1-100\n");

        return 2;
    }

    if (strcmp(argv[2], "cpu") != 0 && strcmp(argv[2], "gpu") != 0 &&
        strcmp(argv[2], "all") != 0) {
        fprintf(stderr, "fan must be cpu, gpu, or all\n");

        return 2;
    }

    snprintf(cmd, sizeof(cmd), "set %s %d\n", argv[2], percent);

    return client_send_command(cmd, false);
}

int client_handle_preset_command(const int argc, char **argv)
{
    char cmd[128];

    if (argc != 3) {
        client_usage(stderr);
        return 2;
    }

    if (strcmp(argv[2], "show") == 0)
        return client_send_command("preset-show\n", false);

    snprintf(cmd, sizeof(cmd), "preset %s\n", argv[2]);

    return client_send_command(cmd, false);
}
