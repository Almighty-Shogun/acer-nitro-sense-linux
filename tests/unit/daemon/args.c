#include "args.h"

#include "daemon/args.h"
#include "core/constants.h"

#include <stdio.h>
#include <string.h>

/**
 * Return whether daemon argument parsing produced a specific status.
 *
 * The tests use daemon_args_parse directly, so this helper keeps status checks
 * readable without hiding the parsed argument object.
 */
static bool daemon_args_status_matches(const int argc, char* const argv[], struct daemon_args* args, const int expected)
{
    return daemon_args_parse(argc, argv, args) == expected;
}

/**
 * Verify default daemon arguments.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int expect_default_args(void)
{
    struct daemon_args args;

    char arg0[] = "acer-nitro-sensed";

    char* const argv[] = {arg0};

    const bool parse_ok = daemon_args_status_matches(1, argv, &args, 0);
    const bool config_ok = strcmp(args.config_path, ANS_DEFAULT_CONFIG) == 0;

    const bool default_flags_ok = !args.config_path_explicit
                                  && !args.force_model
                                  && !args.check_config
                                  && !args.probe
                                  && !args.validate;

    const bool args_ok = parse_ok && config_ok && default_flags_ok;

    if (!args_ok)
    {
        fprintf(stderr, "unit-test failed: daemon args defaults\n");

        return 1;
    }

    return 0;
}

/**
 * Verify daemon option parsing.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
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

    char* const argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};

    const bool parse_ok = daemon_args_status_matches(7, argv, &args, 0);
    const bool config_ok = strcmp(args.config_path, "/tmp/model.json") == 0 && args.config_path_explicit;

    const bool flags_ok = args.force_model && args.check_config && args.probe && args.validate;

    const bool args_ok = parse_ok && config_ok && flags_ok;

    if (!args_ok)
    {
        fprintf(stderr, "unit-test failed: daemon args config and flags\n");

        return 1;
    }

    return 0;
}

/**
 * Verify daemon help parsing.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int expect_help(void)
{
    struct daemon_args args;

    char arg0[] = "acer-nitro-sensed";
    char arg1[] = "--help";

    char* const argv[] = {arg0, arg1};

    const bool help_ok = daemon_args_status_matches(2, argv, &args, 1);

    if (!help_ok)
    {
        fprintf(stderr, "unit-test failed: daemon args help\n");

        return 1;
    }

    return 0;
}

/**
 * Verify invalid daemon argument handling.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int expect_invalid_args(void)
{
    int failures = 0;
    struct daemon_args args;

    char unknown_arg0[] = "acer-nitro-sensed";
    char unknown_arg1[] = "--unknown";
    char missing_arg0[] = "acer-nitro-sensed";
    char missing_arg1[] = "--config";

    char* const unknown[] = {unknown_arg0, unknown_arg1};
    char* const missing_config[] = {missing_arg0, missing_arg1};

    const bool unknown_option_rejected = daemon_args_status_matches(2, unknown, &args, -1);

    if (!unknown_option_rejected)
    {
        fprintf(stderr, "unit-test failed: daemon args unknown option\n");

        failures++;
    }

    const bool missing_config_rejected = daemon_args_status_matches(2, missing_config, &args, -1);

    if (!missing_config_rejected)
    {
        fprintf(stderr, "unit-test failed: daemon args missing config value\n");

        failures++;
    }

    return failures;
}

/**
 * Run daemon argument tests.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_daemon_args(void)
{
    int failures = 0;

    failures += expect_default_args();
    failures += expect_config_and_flags();
    failures += expect_help();
    failures += expect_invalid_args();

    return failures;
}
