#include "unit/helpers.h"

#include "commands/daemon/daemon.h"

#include <string.h>
#include <unistd.h>

/**
 * Read a command reply through a pipe.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int read_pipe_reply(const int fd, char* out, const size_t out_len)
{
    size_t used = 0;
    bool overflow = false;

    if (!out || out_len == 0)
        return -1;

    out[0] = '\0';

    for (;;)
    {
        char chunk[256];
        const ssize_t n = read(fd, chunk, sizeof(chunk));

        if (n < 0)
            return -1;

        if (n == 0) break;

        size_t copy_len = (size_t)n;
        const size_t available = out_len - 1 - used;

        if (copy_len > available)
        {
            copy_len = available;
            overflow = true;
        }

        if (copy_len > 0)
        {
            memcpy(out + used, chunk, copy_len);

            used += copy_len;

            out[used] = '\0';
        }
    }

    return overflow ? -1 : 0;
}

/**
 * Execute a void reply helper.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_read_reply(
    void (*reply_fn)(int, const struct ans_config*, const fan_state [ANS_MAX_FANS], bool, const char*),
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode,
    const char* preset,
    char* out,
    const size_t out_len
)
{
    int fds[2];

    if (pipe(fds) < 0)
        return -1;

    reply_fn(fds[1], cfg, states, auto_mode, preset);

    close(fds[1]);

    const int result = read_pipe_reply(fds[0], out, out_len);

    close(fds[0]);

    return result;
}

/**
 * Execute a daemon command in the unit harness.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_execute_command(
    const char* cmd,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled,
    const bool can_control,
    char* out,
    const size_t out_len
)
{
    int fds[2];

    daemon_runtime_state runtime = {
        .power_source_auto_apply = cfg->power_source_profiles.auto_apply,
    };

    if (pipe(fds) < 0)
        return -1;

    execute_command(fds[1], ec, cfg, states, auto_mode, preset, preset_len, coolboost_enabled, &runtime, cmd, can_control);

    close(fds[1]);

    const int result = read_pipe_reply(fds[0], out, out_len);

    close(fds[0]);

    return result;
}
