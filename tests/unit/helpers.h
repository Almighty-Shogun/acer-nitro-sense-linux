#ifndef ANS_UNIT_HELPERS_H
#define ANS_UNIT_HELPERS_H

#include "daemon/types.h"

/**
 * Capture output from a daemon reply helper.
 *
 * Reply helpers write to a file descriptor, so tests use this wrapper to
 * inspect the generated text.
 */
int unit_read_reply(
    void (*reply_fn)(int, const struct ans_config*, const fan_state [ANS_MAX_FANS], bool, const char*),
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    char* out,
    size_t out_len
);

/**
 * Execute a daemon command and capture the reply.
 *
 * Command tests use this to exercise the same registry path as the daemon
 * control socket.
 */
int unit_execute_command(
    const char* cmd,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    bool can_control,
    char* out,
    size_t out_len
);


#endif
