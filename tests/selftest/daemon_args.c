#include "selftest/daemon_args.h"

#include "core/constants.h"
#include "daemon/args.h"

#include <stdio.h>
#include <string.h>

static int expect_default_args(void)
{
    struct daemon_args args;
    char *argv[] = {"acer-nitro-sensed"};

    if (daemon_args_parse(1, argv, &args) != 0 ||
        strcmp(args.config_path, ANS_DEFAULT_CONFIG) != 0 ||
        args.config_path_explicit ||
        args.force_model || args.check_config || args.probe || args.validate) {
        fprintf(stderr, "self-test failed: daemon args defaults\n");
        return 1;
    }

    return 0;
}

static int expect_config_and_flags(void)
{
    struct daemon_args args;
    char *argv[] = {
        "acer-nitro-sensed",
        "--config", "/tmp/model.json",
        "--force-model",
        "--check-config",
        "--probe",
        "--validate-model",
    };

    if (daemon_args_parse(7, argv, &args) != 0 ||
        strcmp(args.config_path, "/tmp/model.json") != 0 ||
        !args.config_path_explicit ||
        !args.force_model || !args.check_config || !args.probe ||
        !args.validate) {
        fprintf(stderr, "self-test failed: daemon args config and flags\n");
        return 1;
    }

    return 0;
}

static int expect_help(void)
{
    struct daemon_args args;
    char *argv[] = {"acer-nitro-sensed", "--help"};

    if (daemon_args_parse(2, argv, &args) != 1) {
        fprintf(stderr, "self-test failed: daemon args help\n");
        return 1;
    }

    return 0;
}

static int expect_invalid_args(void)
{
    int failures = 0;
    struct daemon_args args;
    char *unknown[] = {"acer-nitro-sensed", "--unknown"};
    char *missing_config[] = {"acer-nitro-sensed", "--config"};

    if (daemon_args_parse(2, unknown, &args) != -1) {
        fprintf(stderr, "self-test failed: daemon args unknown option\n");
        failures++;
    }
    if (daemon_args_parse(2, missing_config, &args) != -1) {
        fprintf(stderr, "self-test failed: daemon args missing config value\n");
        failures++;
    }

    return failures;
}

int selftest_run_daemon_args(void)
{
    int failures = 0;

    failures += expect_default_args();
    failures += expect_config_and_flags();
    failures += expect_help();
    failures += expect_invalid_args();
    return failures;
}
