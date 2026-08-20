#ifndef ANS_SELFTEST_HELPERS_H
#define ANS_SELFTEST_HELPERS_H

#include "ans.h"
#include "daemon/types.h"

#include <stddef.h>

int selftest_read_reply(void (*reply_fn)(int, const struct ans_config *,
                                         const fan_state[ANS_MAX_FANS],
                                         bool, const char *),
                        const struct ans_config *cfg,
                        const fan_state states[ANS_MAX_FANS],
                        bool auto_mode, const char *preset,
                        char *out, size_t out_len);
int selftest_execute_command(const char *cmd, struct ec_device *ec,
                             const struct ans_config *cfg,
                             fan_state states[ANS_MAX_FANS],
                             bool *auto_mode, char *preset,
                             size_t preset_len, bool *coolboost_enabled,
                             bool can_control, char *out, size_t out_len);

#endif
