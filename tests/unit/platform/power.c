#include "cases.h"

#include "ec/ec.h"
#include "../helpers.h"

#include <stdio.h>
#include <stdlib.h>
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
} power_source_command_context;

/**
 * Execute one power-source command against the shared daemon context.
 *
 * Power-source command tests all use the same command dispatcher path, so this
 * wrapper keeps the individual assertions focused on reply and EC state.
 */
static bool execute_power_source_command(const power_source_command_context* ctx, const char* command, const bool privileged)
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
        privileged,
        ctx->reply,
        ctx->reply_len
    );

    return unit_command >= 0;
}

/**
 * Return whether the platform profile register matches the expected value.
 *
 * The fake EC exposes the same profile register used by the daemon, so profile
 * application is validated through the EC-facing side effect.
 */
static bool platform_profile_register_matches(struct ec_device* ec, const struct ans_config* cfg, const int expected_value)
{
    return ec_read_byte(ec, cfg->platform_profiles.reg) == expected_value;
}

/**
 * Return whether battery status reports the quiet target profile.
 *
 * Status is unprivileged and should describe the profile that would be applied
 * for the currently detected power source.
 */
static bool battery_status_command_matches(power_source_command_context* ctx)
{
    const bool executed = execute_power_source_command(ctx, "power-source status\n", false);

    const bool source_ok = strstr(ctx->reply, "power_source=battery") != NULL;
    const bool target_ok = strstr(ctx->reply, "target_profile=quiet") != NULL;

    return executed && source_ok && target_ok;
}

/**
 * Return whether an unprivileged power-source command was denied.
 *
 * Profile changes mutate EC state and must not be accepted from unprivileged
 * command paths.
 */
static bool denied_power_source_command_matches(power_source_command_context* ctx, const char* command)
{
    const bool executed = execute_power_source_command(ctx, command, false);
    const bool reply_ok = strstr(ctx->reply, "error permission denied") != NULL;

    return executed && reply_ok;
}

/**
 * Return whether applying the battery profile selected quiet mode.
 *
 * Battery power maps to the quiet platform profile in the fake model used by
 * these command tests.
 */
static bool battery_apply_command_matches(power_source_command_context* ctx)
{
    const bool executed = execute_power_source_command(ctx, "power-source apply\n", true);

    const bool reply_ok = strstr(ctx->reply, "power_source=battery profile=quiet") != NULL;

    const bool register_ok = platform_profile_register_matches(ctx->ec, ctx->cfg, 0x00);

    return executed && reply_ok && register_ok;
}

/**
 * Return whether enabling auto-apply immediately applied the battery profile.
 *
 * When auto-apply is enabled on battery, the daemon should apply quiet mode and
 * report that auto profile switching is on.
 */
static bool battery_auto_on_command_matches(power_source_command_context* ctx)
{
    const bool executed = execute_power_source_command(ctx, "power-source auto on\n", true);

    const bool reply_ok = strstr(ctx->reply, "power_source=battery auto_apply=on profile=quiet") != NULL;

    const bool register_ok = platform_profile_register_matches(ctx->ec, ctx->cfg, 0x00);

    return executed && reply_ok && register_ok;
}

/**
 * Return whether disabling auto-apply preserves the current battery profile.
 *
 * Turning auto-apply off should be reported without changing the already
 * selected quiet profile.
 */
static bool battery_auto_off_command_matches(power_source_command_context* ctx)
{
    const bool executed = execute_power_source_command(ctx, "power-source auto off\n", true);

    const bool reply_ok = strstr(ctx->reply, "power_source=battery auto_apply=off profile=quiet") != NULL;

    return executed && reply_ok;
}

/**
 * Return whether applying the AC profile selected balanced mode.
 *
 * AC power maps to the balanced platform profile in the fake model used by
 * these command tests.
 */
static bool ac_apply_command_matches(power_source_command_context* ctx)
{
    const bool executed = execute_power_source_command(ctx, "power-source apply\n", true);

    const bool reply_ok = strstr(ctx->reply, "power_source=ac profile=balanced") != NULL;

    const bool register_ok = platform_profile_register_matches(ctx->ec, ctx->cfg, 0x01);

    return executed && reply_ok && register_ok;
}

/**
 * Verify power-source profile commands.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_platform_power_source_commands(
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

    power_source_command_context ctx = {
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

    setenv("ANS_FAKE_POWER_SOURCE", "battery", 1);

    const bool battery_status_ok = battery_status_command_matches(&ctx);

    if (!battery_status_ok)
    {
        fprintf(stderr, "unit-test failed: power-source status command path\n");

        failures++;
    }

    const bool denied_apply_ok = denied_power_source_command_matches(&ctx, "power-source apply\n");

    if (!denied_apply_ok)
    {
        fprintf(stderr, "unit-test failed: denied power-source apply command path\n");

        failures++;
    }

    const bool denied_auto_ok = denied_power_source_command_matches(&ctx, "power-source auto on\n");

    if (!denied_auto_ok)
    {
        fprintf(stderr, "unit-test failed: denied power-source auto command path\n");

        failures++;
    }

    const bool battery_apply_ok = battery_apply_command_matches(&ctx);

    if (!battery_apply_ok)
    {
        fprintf(stderr, "unit-test failed: power-source battery apply command path\n");

        failures++;
    }

    const bool battery_auto_on_ok = battery_auto_on_command_matches(&ctx);

    if (!battery_auto_on_ok)
    {
        fprintf(stderr, "unit-test failed: power-source auto on command path\n");

        failures++;
    }

    const bool battery_auto_off_ok = battery_auto_off_command_matches(&ctx);

    if (!battery_auto_off_ok)
    {
        fprintf(stderr, "unit-test failed: power-source auto off command path\n");

        failures++;
    }

    setenv("ANS_FAKE_POWER_SOURCE", "ac", 1);

    const bool ac_apply_ok = ac_apply_command_matches(&ctx);

    if (!ac_apply_ok)
    {
        fprintf(stderr, "unit-test failed: power-source AC apply command path\n");

        failures++;
    }

    unsetenv("ANS_FAKE_POWER_SOURCE");

    return failures;
}
