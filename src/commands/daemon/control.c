#include "commands/daemon/control.h"

#include "fan/control.h"
#include "control/protocol.h"
#include "platform/control.h"
#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Carries daemon lifecycle command dependencies.
 *
 * A context keeps the command table compact without forcing every handler to
 * accept each daemon dependency as a separate parameter.
 */
typedef struct
{
    int client;
    struct ec_device* ec;
    const struct ans_config* cfg;
    fan_state* states;
    bool auto_mode;
    const char* preset;
} daemon_control_context;

/**
 * Handles one daemon lifecycle command.
 *
 * Lifecycle handlers share a context because resume needs EC and fan state,
 * while stop only needs the client and daemon run flag.
 */
typedef bool (*daemon_control_handler)(const daemon_control_context* ctx, const char* cmd);

/**
 * Maps a lifecycle command name to its daemon handler.
 *
 * Keeping resume and stop in a table gives the top-level registry a single
 * control-command entry point.
 */
typedef struct
{
    const char* name;
    daemon_control_handler handler;
} daemon_control_command;

/**
 * Reapply daemon-owned EC state after resume.
 *
 * Resume restores init writes, wakes configured sensors, and reapplies either
 * firmware-auto mode or the current daemon fan-control state.
 */
static bool handle_resume_command(const daemon_control_context* ctx, const char* cmd)
{
    if (!command_is_exact(cmd, "resume"))
    {
        control_reply(ctx->client, "error usage: resume\n");

        return true;
    }

    apply_init_writes(ctx->ec, ctx->cfg);
    apply_sensor_power_control(ctx->cfg, "on");

    if (firmware_auto_mode(ctx->auto_mode, ctx->preset))
    {
        apply_firmware_auto_fan_mode(ctx->ec, ctx->cfg);
    }
    else
    {
        apply_current_control_state(ctx->ec, ctx->cfg, ctx->states);
    }

    if (!daemon_quiet_logs)
        fprintf(stderr, "resume_reapply mode=%s preset=%s\n", control_mode(ctx->auto_mode, ctx->preset), ctx->preset);

    control_reply(ctx->client, "resume=ok mode=%s preset=%s\n", control_mode(ctx->auto_mode, ctx->preset), ctx->preset);

    return true;
}

/**
 * Request daemon shutdown from the control socket.
 *
 * The polling loop observes `daemon_running` and performs the normal firmware
 * reset path during service shutdown.
 */
static bool handle_stop_command(const daemon_control_context* ctx, const char* cmd)
{
    if (!command_is_exact(cmd, "stop"))
    {
        control_reply(ctx->client, "error usage: stop\n");

        return true;
    }

    daemon_running = 0;

    control_reply(ctx->client, "stop=ok reset=firmware\n");

    return true;
}

/**
 * Supported daemon lifecycle commands.
 *
 * Lifecycle routing is intentionally small because these commands affect the
 * daemon process itself rather than a single feature module.
 */
static const daemon_control_command DAEMON_CONTROL_COMMANDS[] = {
    {.name = "resume", .handler = handle_resume_command},
    {.name = "stop", .handler = handle_stop_command},
};

/**
 * Find a daemon lifecycle command entry.
 *
 * Lifecycle command routing is table-driven so command names and behavior stay
 * visible in one place.
 */
static const daemon_control_command* find_daemon_control_command(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(DAEMON_CONTROL_COMMANDS); i++)
    {
        if (strcmp(name, DAEMON_CONTROL_COMMANDS[i].name) == 0)
            return &DAEMON_CONTROL_COMMANDS[i];
    }

    return NULL;
}

/**
 * Dispatch daemon lifecycle commands.
 *
 * The top-level registry has already selected this module; this function only
 * chooses the concrete lifecycle action or returns an unknown-command reply.
 */
bool handle_daemon_control_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    const char* cmd
)
{
    char command[32];

    const daemon_control_context context = {
        .client = client,
        .ec = ec,
        .cfg = cfg,
        .states = states,
        .auto_mode = auto_mode,
        .preset = preset,
    };

    if (!command_first_token(cmd, command, sizeof(command)))
    {
        control_reply(client, "error unknown command\n");

        return true;
    }

    const daemon_control_command* entry = find_daemon_control_command(command);

    if (entry)
        return entry->handler(&context, cmd);

    control_reply(client, "error unknown command\n");

    return true;
}
