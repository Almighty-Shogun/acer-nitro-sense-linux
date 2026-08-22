#include "args.h"

#include "core/constants.h"
#include "daemon/args.h"

#include <stdio.h>
#include <string.h>

static int expect_default_args(void)
{
    struct daemon_args args;
    char arg0[] = "acer-nitro-sensed";
    char *const argv[] = {arg0};

    if (daemon_args_parse(1, argv, &args) != 0 ||
        strcmp(args.config_path, ANS_DEFAULT_CONFIG) != 0 ||
        args.config_path_explicit ||
        args.force_model || args.check_config || args.probe || args.validate) {
        fprintf(stderr, "unit-test failed: daemon args defaults\n");
        return 1;
    }

    return 0;
}

static int expect_config_and_flags(void)
{
    struct daemon_args args;
    char arg0[] = "acer-nitro-sensed";
    char arg1[] = "--config";
    char arg2[] = "/tmp/model.json";
    char arg3[] = "--force-model";
    char arg4[] = "--check-config";
    char arg5[] = "--probe";
    char arg6[] = "--validate-model";
    char *const argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};

    if (daemon_args_parse(7, argv, &args) != 0 ||
        strcmp(args.config_path, "/tmp/model.json") != 0 ||
        !args.config_path_explicit ||
        !args.force_model || !args.check_config || !args.probe ||
        !args.validate) {
        fprintf(stderr, "unit-test failed: daemon args config and flags\n");
        return 1;
    }

    return 0;
}

static int expect_help(void)
{
    struct daemon_args args;
    char arg0[] = "acer-nitro-sensed";
    char arg1[] = "--help";
    char *const argv[] = {arg0, arg1};

    if (daemon_args_parse(2, argv, &args) != 1) {
        fprintf(stderr, "unit-test failed: daemon args help\n");
        return 1;
    }

    return 0;
}

static int expect_invalid_args(void)
{
    int failures = 0;
    struct daemon_args args;
    char unknown_arg0[] = "acer-nitro-sensed";
    char unknown_arg1[] = "--unknown";
    char missing_arg0[] = "acer-nitro-sensed";
    char missing_arg1[] = "--config";
    char *const unknown[] = {unknown_arg0, unknown_arg1};
    char *const missing_config[] = {missing_arg0, missing_arg1};

    if (daemon_args_parse(2, unknown, &args) != -1) {
        fprintf(stderr, "unit-test failed: daemon args unknown option\n");
        failures++;
    }
    if (daemon_args_parse(2, missing_config, &args) != -1) {
        fprintf(stderr, "unit-test failed: daemon args missing config value\n");
        failures++;
    }

    return failures;
}

int unit_run_daemon_args(void)
{
    int failures = 0;

    failures += expect_default_args();
    failures += expect_config_and_flags();
    failures += expect_help();
    failures += expect_invalid_args();
    return failures;
}
