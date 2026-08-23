#include "cases.h"

#include "ec/ec.h"
#include "../helpers.h"
#include "fan/control.h"

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
} invalid_command_context;

/**
 * Execute an invalid command and match the expected error reply.
 *
 * Invalid command tests all share one runtime context; only the command text and
 * expected error differ between assertions.
 */
static bool invalid_command_matches(invalid_command_context* ctx, const char* command, const char* expected)
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
        true,
        ctx->reply,
        ctx->reply_len
    );

    return result >= 0 && strstr(ctx->reply, expected);
}

/**
 * Verify invalid command handling and reset behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_invalid_and_reset_commands(
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

    invalid_command_context ctx = {
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

    const bool unknown_preset_ok = invalid_command_matches(&ctx, "preset nope\n", "error unknown preset");

    if (!unknown_preset_ok)
    {
        fprintf(stderr, "unit-test failed: unknown preset command path\n");

        failures++;
    }

    const bool malformed_set_ok = invalid_command_matches(&ctx, "set gpu 101\n", "error usage: set cpu|gpu|all 1-100");

    if (!malformed_set_ok)
    {
        fprintf(stderr, "unit-test failed: malformed set command path\n");

        failures++;
    }

    const bool strict_status_ok = invalid_command_matches(&ctx, "status extra\n", "error usage: status");

    if (!strict_status_ok)
    {
        fprintf(stderr, "unit-test failed: strict status command path\n");

        failures++;
    }

    states[0].percent = 74;
    states[1].percent = 68;

    apply_current_control_state(ec, cfg, states);
    apply_reset_writes(ec, cfg);

    const bool cpu_reset_ok = ec_read_byte(ec, cfg->fans[0].write_register) == cfg->fans[0].reset_speed;
    const bool gpu_reset_ok = ec_read_byte(ec, cfg->fans[1].write_register) == cfg->fans[1].reset_speed;

    const bool reset_ok = cpu_reset_ok && gpu_reset_ok;

    if (!reset_ok)
    {
        fprintf(stderr, "unit-test failed: reset writes\n");

        failures++;
    }

    return failures;
}
