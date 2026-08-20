#include "selftest/helpers.h"

#include "commands/daemon.h"

#include <unistd.h>

int selftest_read_reply(void (*reply_fn)(int, const struct ans_config *,
                                         const fan_state[ANS_MAX_FANS],
                                         bool, const char *),
                        const struct ans_config *cfg,
                        const fan_state states[ANS_MAX_FANS],
                        const bool auto_mode, const char *preset,
                        char *out, const size_t out_len)
{
    int fds[2];
    ssize_t n;

    if (pipe(fds) < 0)
        return -1;

    reply_fn(fds[1], cfg, states, auto_mode, preset);
    close(fds[1]);
    n = read(fds[0], out, out_len - 1);
    close(fds[0]);
    if (n < 0)
        return -1;

    out[n] = '\0';
    return 0;
}

int selftest_execute_command(const char *cmd, struct ec_device *ec,
                             const struct ans_config *cfg,
                             fan_state states[ANS_MAX_FANS],
                             bool *auto_mode, char *preset,
                             const size_t preset_len,
                             bool *coolboost_enabled,
                             const bool can_control, char *out,
                             const size_t out_len)
{
    int fds[2];
    ssize_t n;
    daemon_runtime_state runtime = {
        .power_source_auto_apply = cfg->power_source_profiles.auto_apply,
    };

    if (pipe(fds) < 0)
        return -1;

    execute_command(fds[1], ec, cfg, states, auto_mode, preset, preset_len,
                    coolboost_enabled, &runtime, cmd, can_control);
    close(fds[1]);
    n = read(fds[0], out, out_len - 1);
    close(fds[0]);
    if (n < 0)
        return -1;

    out[n] = '\0';
    return 0;
}
