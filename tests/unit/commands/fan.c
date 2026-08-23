#include "cases.h"

#include "ec/ec.h"
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
} fan_socket_context;

/**
 * Execute a fan command and match its reply.
 *
 * Fan socket tests all share the same runtime state. This helper keeps the
 * repeated command dispatch arguments out of the individual assertions.
 */
static bool fan_socket_command_matches(fan_socket_context* ctx, const char* command, const bool privileged, const char* expected)
{
    const int result = unit_execute_command(
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

    return result >= 0 && strstr(ctx->reply, expected);
}

/**
 * Verify fan command socket behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_fan_socket_commands(
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
    fan_socket_context ctx = {
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

    const bool denied_set_ok = fan_socket_command_matches(&ctx, "set cpu 44\n", false, "error permission denied");

    if (!denied_set_ok)
    {
        fprintf(stderr, "unit-test failed: denied set command path\n");

        failures++;
    }

    const bool set_cpu_reply_ok = fan_socket_command_matches(&ctx, "set cpu 44\n", true, "mode=manual fan=cpu requested=44");

    const bool set_cpu_ec_ok = ec_read_byte(ec, cfg->fans[0].write_register) == 44;
    const bool set_cpu_state_ok = !*auto_mode && strcmp(preset, "manual") == 0 && states[0].percent == 44;

    const bool set_cpu_ok = set_cpu_reply_ok && set_cpu_state_ok && set_cpu_ec_ok;

    if (!set_cpu_ok)
    {
        fprintf(stderr, "unit-test failed: set cpu command path\n");

        failures++;
    }

    const bool set_all_reply_ok = fan_socket_command_matches(&ctx, "set all 48\n", true, "mode=manual fan=all requested=48");

    const bool set_all_state_ok = states[0].percent == 48 && states[1].percent == 48;

    const bool set_all_ec_ok = ec_read_byte(ec, cfg->fans[0].write_register) == 48
                               && ec_read_byte(ec, cfg->fans[1].write_register) == 48;

    const bool set_all_ok = set_all_reply_ok && set_all_state_ok && set_all_ec_ok;

    if (!set_all_ok)
    {
        fprintf(stderr, "unit-test failed: set all command path\n");

        failures++;
    }

    const bool preset_reply_ok = fan_socket_command_matches(&ctx, "preset max\n", true, "mode=preset preset=max cpu=100 gpu=100");

    const bool preset_state_ok = !*auto_mode && strcmp(preset, "max") == 0 && states[0].percent == 100 && states[1].percent == 100;
    const bool preset_ok = preset_reply_ok && preset_state_ok;

    if (!preset_ok)
    {
        fprintf(stderr, "unit-test failed: preset command path\n");

        failures++;
    }

    const bool auto_reply_ok = fan_socket_command_matches(&ctx, "auto\n", true, "mode=auto preset=auto");

    const bool auto_state_ok = *auto_mode && strcmp(preset, "auto") == 0 && states[0].percent == 50 && states[1].percent == 50;
    const bool auto_ok = auto_reply_ok && auto_state_ok;

    if (!auto_ok)
    {
        fprintf(stderr, "unit-test failed: auto command path\n");

        failures++;
    }

    return failures;
}
