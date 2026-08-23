#include "cases.h"

#include "../helpers.h"

#include <stdio.h>
#include <signal.h>
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
} daemon_lifecycle_context;

/**
 * Execute one daemon command through the test command runner.
 *
 * The lifecycle tests only care about daemon command text and reply contents,
 * so this wrapper keeps the shared EC/config/runtime arguments out of each
 * assertion.
 */
static bool daemon_lifecycle_command_matches(daemon_lifecycle_context* ctx, const char* command, const char* expected)
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
 * Verify daemon lifecycle commands.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_daemon_lifecycle_commands(
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

    daemon_lifecycle_context ctx = {
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

    const bool resume_ok = daemon_lifecycle_command_matches(&ctx, "resume\n", "resume=ok mode=auto preset=auto");

    if (!resume_ok)
    {
        fprintf(stderr, "unit-test failed: resume command path\n");

        failures++;
    }

    daemon_running = 1;

    const bool stop_reply_ok = daemon_lifecycle_command_matches(&ctx, "stop\n", "stop=ok reset=firmware");
    const bool stop_ok = stop_reply_ok && daemon_running == 0;

    if (!stop_ok)
    {
        fprintf(stderr, "unit-test failed: stop command path\n");

        failures++;
    }

    daemon_running = 1;

    return failures;
}
