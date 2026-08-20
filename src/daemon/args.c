#include "daemon/args.h"

#include "core/constants.h"

#include <stdio.h>
#include <string.h>

static void daemon_usage(FILE *out)
{
    fprintf(out, "usage: acer-nitro-sensed [--config PATH] [--force-model] [--check-config] [--probe] [--validate-model]\n");
}

void daemon_args_init(struct daemon_args *args)
{
    args->config_path = ANS_DEFAULT_CONFIG;
    args->config_path_explicit = false;
    args->force_model = false;
    args->check_config = false;
    args->probe = false;
    args->validate = false;
}

int daemon_args_parse(const int argc, char **argv, struct daemon_args *args)
{
    daemon_args_init(args);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            args->config_path = argv[++i];
            args->config_path_explicit = true;
        } else if (strcmp(argv[i], "--force-model") == 0) {
            args->force_model = true;
        } else if (strcmp(argv[i], "--check-config") == 0) {
            args->check_config = true;
        } else if (strcmp(argv[i], "--probe") == 0) {
            args->probe = true;
        } else if (strcmp(argv[i], "--validate-model") == 0) {
            args->validate = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            daemon_usage(stdout);
            return 1;
        } else {
            daemon_usage(stderr);
            return -1;
        }
    }

    return 0;
}
