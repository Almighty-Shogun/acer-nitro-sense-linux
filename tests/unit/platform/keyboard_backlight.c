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
} keyboard_backlight_command_context;

/**
 * Execute one keyboard-backlight command against the shared daemon context.
 *
 * Each keyboard-backlight test uses the same socket command path, so this
 * wrapper keeps the expected reply content separate from command plumbing.
 */
static bool execute_keyboard_backlight_command(
    const keyboard_backlight_command_context* ctx,
    const char* command,
    const bool privileged
)
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
 * Return whether the EC keyboard-backlight status reply is complete.
 *
 * Status should expose availability, backend, the active EC register, and the
 * current timeout state without requiring privileged access.
 */
static bool keyboard_backlight_status_command_matches(keyboard_backlight_command_context* ctx)
{
    const bool executed = execute_keyboard_backlight_command(ctx, "keyboard-backlight status\n", false);

    const bool available_ok = strstr(ctx->reply, "keyboard_backlight=available") != NULL;
    const bool backend_ok = strstr(ctx->reply, "backend=ec") != NULL;
    const bool register_ok = strstr(ctx->reply, "register=0x31") != NULL;
    const bool timeout_ok = strstr(ctx->reply, "timeout=off") != NULL;

    return executed && available_ok && backend_ok && register_ok && timeout_ok;
}

/**
 * Return whether setting keyboard-backlight brightness reports the EC level.
 *
 * The model exposes five logical steps, so 75 percent should map to brightness
 * level 3 in the command reply.
 */
static bool keyboard_backlight_set_command_matches(keyboard_backlight_command_context* ctx)
{
    const bool executed = execute_keyboard_backlight_command(ctx, "keyboard-backlight set 75\n", true);

    const bool available_ok = strstr(ctx->reply, "keyboard_backlight=available") != NULL;
    const bool brightness_ok = strstr(ctx->reply, "brightness=3") != NULL;
    const bool percent_ok = strstr(ctx->reply, "percent=75") != NULL;

    return executed && available_ok && brightness_ok && percent_ok;
}

/**
 * Return whether enabling the keyboard-backlight timeout reports 30 seconds.
 *
 * Acer's timeout behavior is represented by a boolean command with a fixed
 * timeout value in the model profile.
 */
static bool keyboard_backlight_timeout_on_command_matches(keyboard_backlight_command_context* ctx)
{
    const bool executed = execute_keyboard_backlight_command(ctx, "keyboard-backlight timeout on\n", true);

    const bool timeout_ok = strstr(ctx->reply, "keyboard_backlight_timeout=on") != NULL;
    const bool seconds_ok = strstr(ctx->reply, "timeout_seconds=30") != NULL;

    return executed && timeout_ok && seconds_ok;
}

/**
 * Return whether disabling the keyboard-backlight timeout is reported.
 *
 * The command should switch the timeout flag off while leaving brightness
 * control available.
 */
static bool keyboard_backlight_timeout_off_command_matches(keyboard_backlight_command_context* ctx)
{
    const bool executed = execute_keyboard_backlight_command(ctx, "keyboard-backlight timeout off\n", true);

    const bool timeout_ok = strstr(ctx->reply, "keyboard_backlight_timeout=off") != NULL;

    return executed && timeout_ok;
}

/**
 * Verify keyboard backlight commands.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_platform_keyboard_backlight_commands(
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

    keyboard_backlight_command_context ctx = {
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

    const bool status_ok = keyboard_backlight_status_command_matches(&ctx);

    if (!status_ok)
    {
        fprintf(stderr, "unit-test failed: keyboard-backlight ec status command path\n");

        failures++;
    }

    const bool set_ok = keyboard_backlight_set_command_matches(&ctx);

    if (!set_ok)
    {
        fprintf(stderr, "unit-test failed: keyboard-backlight ec set command path\n");

        failures++;
    }

    const bool timeout_on_ok = keyboard_backlight_timeout_on_command_matches(&ctx);

    if (!timeout_on_ok)
    {
        fprintf(stderr, "unit-test failed: keyboard-backlight timeout on command path\n");

        failures++;
    }

    const bool timeout_off_ok = keyboard_backlight_timeout_off_command_matches(&ctx);

    if (!timeout_off_ok)
    {
        fprintf(stderr, "unit-test failed: keyboard-backlight timeout off command path\n");

        failures++;
    }

    return failures;
}
