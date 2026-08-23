#include "cases.h"

#include "../helpers.h"

#include <stdio.h>
#include <string.h>

typedef struct
{
    struct ec_device* ec;
    const struct ans_config* cfg;
    fan_state* states;
    bool* auto_mode;
    char* preset;
    size_t preset_len;
    bool* coolboost_enabled;
    char* reply;
    size_t reply_len;
} platform_status_command_context;

/**
 * Execute one read-only platform status command.
 *
 * These commands all use the same unprivileged command dispatcher path, so this
 * helper keeps reply assertions separate from command plumbing.
 */
static bool execute_platform_status_command(const platform_status_command_context* ctx, const char* command)
{
    const int unit_command = unit_execute_command(
        command,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        ctx->auto_mode,
        ctx->preset,
        ctx->preset_len,
        ctx->coolboost_enabled,
        false,
        ctx->reply,
        ctx->reply_len
    );

    return unit_command >= 0;
}

/**
 * Return whether the primary status reply exposes the expected model state.
 *
 * The status command is the compact human-readable summary consumed by both the
 * CLI and extension-facing socket clients.
 */
static bool status_command_matches(platform_status_command_context* ctx)
{
    const bool executed = execute_platform_status_command(ctx, "status\n");

    const bool model_ok = strstr(ctx->reply, "model=Acer Nitro Self Test mode=manual") != NULL;
    const bool coolboost_ok = strstr(ctx->reply, "coolboost=off") != NULL;

    return executed && model_ok && coolboost_ok;
}

/**
 * Return whether the CoolBoost status command reports the current flag.
 *
 * This read-only command should be available without privileged access.
 */
static bool coolboost_status_command_matches(platform_status_command_context* ctx)
{
    const bool executed = execute_platform_status_command(ctx, "coolboost status\n");

    const bool reply_ok = strstr(ctx->reply, "coolboost=off") != NULL;

    return executed && reply_ok;
}

/**
 * Return whether the capabilities reply exposes the supported platform features.
 *
 * The capabilities command is intentionally broad, so this checks the feature
 * rows that prove fan, profile, and power-source reporting are wired.
 */
static bool capabilities_command_matches(platform_status_command_context* ctx)
{
    const bool executed = execute_platform_status_command(ctx, "capabilities\n");

    const bool fan_control_ok = strstr(ctx->reply, "fan_control=available") != NULL;
    const bool fan_mode_ok = strstr(ctx->reply, "fan_mode=available modes=auto,manual,turbo") != NULL;
    const bool profile_ok = strstr(ctx->reply, "platform_profile=available profiles=quiet,balanced,performance") != NULL;
    const bool power_source_ok = strstr(ctx->reply, "power_source_profile=available auto_apply=off") != NULL;

    return executed && fan_control_ok && fan_mode_ok && profile_ok && power_source_ok;
}

/**
 * Verify read-only platform status commands.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_platform_status_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled
)
{
    int failures = 0;
    char reply[1024];

    platform_status_command_context ctx = {
        .ec = ec,
        .cfg = cfg,
        .states = states,
        .auto_mode = auto_mode,
        .preset = preset,
        .preset_len = preset_len,
        .coolboost_enabled = coolboost_enabled,
        .reply = reply,
        .reply_len = sizeof(reply),
    };

    const bool status_ok = status_command_matches(&ctx);

    if (!status_ok)
    {
        fprintf(stderr, "unit-test failed: status command path\n");

        failures++;
    }

    const bool coolboost_status_ok = coolboost_status_command_matches(&ctx);

    if (!coolboost_status_ok)
    {
        fprintf(stderr, "unit-test failed: coolboost status command path\n");

        failures++;
    }

    const bool capabilities_ok = capabilities_command_matches(&ctx);

    if (!capabilities_ok)
    {
        fprintf(stderr, "unit-test failed: capabilities command path\n");

        failures++;
    }

    return failures;
}
