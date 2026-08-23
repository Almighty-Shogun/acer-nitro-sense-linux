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
} ec_debug_context;

/**
 * Execute an EC command and match its reply.
 *
 * EC debug commands all share the same test runtime state, so this helper keeps
 * each assertion focused on the command text, permission state, and expected
 * reply.
 */
static bool ec_debug_command_matches(ec_debug_context* ctx, const char* command, const bool privileged, const char* expected)
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
 * Verify EC debug commands.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_ec_debug_commands(
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

    ec_debug_context ctx = {
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

    ec_write_byte(ec, 0x40, 0xab);
    ec_write_byte(ec, 0x41, 0xcd);

    const bool read_ok = ec_debug_command_matches(&ctx, "ec-read 0x40\n", true, "ec[0x40]=0xab (171)");

    if (!read_ok)
    {
        fprintf(stderr, "unit-test failed: ec read command path\n");

        failures++;
    }

    const bool dump_ok = ec_debug_command_matches(&ctx, "ec-dump 0x40 0x41\n", true, "0x40: ab cd");

    if (!dump_ok)
    {
        fprintf(stderr, "unit-test failed: ec dump command path\n");

        failures++;
    }

    const bool denied_read_ok = ec_debug_command_matches(
        &ctx,
        "ec-read 0x40\n",
        false,
        "error permission denied"
    );

    if (!denied_read_ok)
    {
        fprintf(stderr, "unit-test failed: denied ec read command path\n");

        failures++;
    }

    return failures;
}
