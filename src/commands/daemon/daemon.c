#include "control/socket.h"
#include "control/protocol.h"
#include "commands/daemon/daemon.h"
#include "commands/daemon/registry.h"

/**
 * Execute one control-socket command.
 *
 * The function builds the shared command context and delegates routing to the
 * registry so permission checks and feature handlers stay centralized.
 */
void execute_command(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime,
    const char* cmd,
    const bool can_control
)
{
    const daemon_command_context ctx = {
        .client = client,
        .ec = ec,
        .cfg = cfg,
        .states = states,
        .auto_mode = auto_mode,
        .preset = preset,
        .preset_len = preset_len,
        .coolboost_enabled = coolboost_enabled,
        .runtime = runtime,
        .cmd = cmd,
        .can_control = can_control,
    };

    dispatch_daemon_command(&ctx);
}

/**
 * Read and handle one connected control client.
 *
 * Invalid socket input receives an immediate protocol error; valid commands
 * are executed with the client's group-based control permission.
 */
void handle_client(
    const int client,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled,
    daemon_runtime_state* runtime
)
{
    char cmd[256];
    const int read_result = control_read_command(client, cmd, sizeof(cmd));

    if (read_result < 0)
    {
        control_reply(client, "error invalid command\n");

        return;
    }

    execute_command(
        client,
        ec,
        cfg,
        states,
        auto_mode,
        preset,
        preset_len,
        coolboost_enabled,
        runtime,
        cmd,
        client_can_control(client)
    );
}
