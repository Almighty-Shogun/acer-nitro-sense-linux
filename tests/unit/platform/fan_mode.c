#include "cases.h"

#include "ec/ec.h"
#include "../helpers.h"
#include "platform/control.h"

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
} fan_mode_command_context;

/**
 * Execute one fan-mode command against the shared daemon command context.
 *
 * Fan-mode commands all exercise the same socket handler path, so this wrapper
 * keeps each test case focused on the command-specific state transition.
 */
static bool execute_fan_mode_command(const fan_mode_command_context* ctx, const char* command, const bool privileged)
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
 * Return whether the fan-mode registers match turbo mode.
 *
 * CoolBoost and the explicit turbo fan-mode command both map to the same EC
 * fan-mode register values.
 */
static bool fan_mode_registers_turbo(struct ec_device* ec, const struct ans_config* cfg)
{
    const bool cpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.cpu_reg) == cfg->fan_modes.cpu_turbo_value;
    const bool gpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.gpu_reg) == cfg->fan_modes.gpu_turbo_value;

    return cpu_mode_ok && gpu_mode_ok;
}

/**
 * Return whether the fan-mode registers match manual mode.
 *
 * Manual mode is used when CoolBoost is disabled and when the explicit manual
 * command returns fan control to normal daemon-owned manual behavior.
 */
static bool fan_mode_registers_manual(struct ec_device* ec, const struct ans_config* cfg)
{
    const bool cpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.cpu_reg) == cfg->fan_modes.cpu_manual_value;
    const bool gpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.gpu_reg) == cfg->fan_modes.gpu_manual_value;

    return cpu_mode_ok && gpu_mode_ok;
}

/**
 * Return whether the fan-mode registers match firmware-auto mode.
 *
 * Firmware-auto mode hands fan control back to the embedded controller, so the
 * daemon should write the profile's auto register values.
 */
static bool fan_mode_registers_auto(struct ec_device* ec, const struct ans_config* cfg)
{
    const bool cpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.cpu_reg) == cfg->fan_modes.cpu_auto_value;
    const bool gpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.gpu_reg) == cfg->fan_modes.gpu_auto_value;

    return cpu_mode_ok && gpu_mode_ok;
}

/**
 * Return whether the denied CoolBoost command reported a permission error.
 *
 * Unprivileged command paths must fail before mutating EC fan-mode registers.
 */
static bool denied_coolboost_command_matches(fan_mode_command_context* ctx)
{
    const bool executed = execute_fan_mode_command(ctx, "coolboost on\n", false);
    const bool reply_ok = strstr(ctx->reply, "error permission denied") != NULL;

    return executed && reply_ok;
}

/**
 * Return whether enabling CoolBoost entered turbo mode.
 *
 * CoolBoost is exposed as a friendly alias for temporary turbo fan behavior.
 */
static bool coolboost_on_command_matches(fan_mode_command_context* ctx)
{
    const bool executed = execute_fan_mode_command(ctx, "coolboost on\n", true);

    const bool reply_ok = strstr(ctx->reply, "coolboost=on") != NULL;
    const bool flag_ok = *ctx->coolboost_enabled;

    const bool registers_ok = fan_mode_registers_turbo(ctx->ec, ctx->cfg);

    return executed && reply_ok && flag_ok && registers_ok;
}

/**
 * Return whether disabling CoolBoost restored manual mode.
 *
 * The command must clear the runtime CoolBoost flag and restore the manual
 * fan-mode EC register values.
 */
static bool coolboost_off_command_matches(fan_mode_command_context* ctx)
{
    const bool executed = execute_fan_mode_command(ctx, "coolboost off\n", true);

    const bool reply_ok = strstr(ctx->reply, "coolboost=off") != NULL;
    const bool flag_ok = !*ctx->coolboost_enabled;

    const bool registers_ok = fan_mode_registers_manual(ctx->ec, ctx->cfg);

    return executed && reply_ok && flag_ok && registers_ok;
}

/**
 * Return whether the turbo fan-mode command entered turbo mode.
 *
 * This pins the canonical fan-mode command separately from the CoolBoost alias.
 */
static bool fan_mode_turbo_command_matches(fan_mode_command_context* ctx)
{
    const bool executed = execute_fan_mode_command(ctx, "fan-mode turbo\n", true);

    const bool reply_ok = strstr(ctx->reply, "fan_mode=turbo") != NULL;
    const bool flag_ok = *ctx->coolboost_enabled;

    const bool registers_ok = fan_mode_registers_turbo(ctx->ec, ctx->cfg);

    return executed && reply_ok && flag_ok && registers_ok;
}

/**
 * Return whether fan-mode status reports the current turbo state.
 *
 * Status must be readable without privileged command access.
 */
static bool fan_mode_status_command_matches(fan_mode_command_context* ctx)
{
    const bool executed = execute_fan_mode_command(ctx, "fan-mode status\n", false);
    const bool reply_ok = strstr(ctx->reply, "fan_mode=turbo") != NULL;

    return executed && reply_ok;
}

/**
 * Return whether the manual fan-mode command restored daemon manual control.
 *
 * Manual mode should clear CoolBoost, clear auto mode, and set the persisted
 * preset name back to manual.
 */
static bool fan_mode_manual_command_matches(fan_mode_command_context* ctx)
{
    const bool executed = execute_fan_mode_command(ctx, "fan-mode manual\n", true);

    const bool reply_ok = strstr(ctx->reply, "fan_mode=manual") != NULL;
    const bool flags_ok = !*ctx->coolboost_enabled && !*ctx->auto_mode;

    const bool preset_ok = strcmp(ctx->preset, "manual") == 0;
    const bool registers_ok = fan_mode_registers_manual(ctx->ec, ctx->cfg);

    return executed && reply_ok && flags_ok && preset_ok && registers_ok;
}

/**
 * Return whether the auto fan-mode command restored firmware-auto control.
 *
 * The command should set the firmware-auto preset while keeping daemon auto
 * mode disabled because the firmware owns the curve.
 */
static bool fan_mode_auto_command_matches(fan_mode_command_context* ctx)
{
    const bool executed = execute_fan_mode_command(ctx, "fan-mode auto\n", true);

    const bool reply_ok = strstr(ctx->reply, "fan_mode=auto") != NULL;
    const bool mode_ok = !*ctx->auto_mode && strcmp(ctx->preset, FIRMWARE_AUTO_PRESET) == 0;
    const bool flag_ok = !*ctx->coolboost_enabled;

    const bool registers_ok = fan_mode_registers_auto(ctx->ec, ctx->cfg);

    return executed && reply_ok && mode_ok && flag_ok && registers_ok;
}

/**
 * Verify firmware fan-mode commands.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_platform_fan_mode_commands(
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

    fan_mode_command_context ctx = {
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

    const bool denied_coolboost_ok = denied_coolboost_command_matches(&ctx);

    if (!denied_coolboost_ok)
    {
        fprintf(stderr, "unit-test failed: denied coolboost command path\n");

        failures++;
    }

    const bool coolboost_on_ok = coolboost_on_command_matches(&ctx);

    if (!coolboost_on_ok)
    {
        fprintf(stderr, "unit-test failed: coolboost on command path\n");

        failures++;
    }

    const bool coolboost_off_ok = coolboost_off_command_matches(&ctx);

    if (!coolboost_off_ok)
    {
        fprintf(stderr, "unit-test failed: coolboost off command path\n");

        failures++;
    }

    const bool fan_mode_turbo_ok = fan_mode_turbo_command_matches(&ctx);

    if (!fan_mode_turbo_ok)
    {
        fprintf(stderr, "unit-test failed: fan-mode turbo command path\n");

        failures++;
    }

    const bool fan_mode_status_ok = fan_mode_status_command_matches(&ctx);

    if (!fan_mode_status_ok)
    {
        fprintf(stderr, "unit-test failed: fan-mode status command path\n");

        failures++;
    }

    const bool fan_mode_manual_ok = fan_mode_manual_command_matches(&ctx);

    if (!fan_mode_manual_ok)
    {
        fprintf(stderr, "unit-test failed: fan-mode manual command path\n");

        failures++;
    }

    const bool fan_mode_auto_ok = fan_mode_auto_command_matches(&ctx);

    if (!fan_mode_auto_ok)
    {
        fprintf(stderr, "unit-test failed: fan-mode auto command path\n");

        failures++;
    }

    return failures;
}
