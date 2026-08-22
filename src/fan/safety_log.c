#include "fan/internal.h"

#include "util/string.h"

#include <stdio.h>
#include <string.h>

static bool text_has_value(const char *text)
{
    return text && text[0] != '\0';
}

static void log_safety_active(const struct fan_config *fan,
                              const fan_state *state,
                              const char *reason,
                              const int requested_percent,
                              const int effective_percent)
{
    if (daemon_quiet_logs)
        return;

    fprintf(stderr,
            "safety active fan=%s reason=%s requested=%d effective=%d temp=%d control_temp=%d rpm=%d ec_read_failures=%d ec_write_failures=%d\n",
            fan->id, reason, requested_percent, effective_percent,
            state->temp_c, state->control_temp_c, state->rpm,
            state->ec_read_failures, state->ec_write_failures);
}

static void log_safety_cleared(const struct fan_config *fan,
                               const fan_state *state,
                               const int requested_percent,
                               const int effective_percent)
{
    if (daemon_quiet_logs)
        return;

    fprintf(stderr,
            "safety cleared fan=%s previous=%s requested=%d effective=%d temp=%d control_temp=%d rpm=%d\n",
            fan->id, state->safety_reason, requested_percent,
            effective_percent, state->temp_c, state->control_temp_c,
            state->rpm);
}

void fan_update_safety_state(const struct fan_config *fan, fan_state *state,
                             const char *reason, const int requested_percent,
                             const int effective_percent)
{
    const bool active = text_has_value(reason);
    const bool changed = active != state->safety_active ||
                         (active && strcmp(state->safety_reason, reason) != 0);

    if (changed) {
        if (active)
            log_safety_active(fan, state, reason, requested_percent,
                              effective_percent);
        else
            log_safety_cleared(fan, state, requested_percent,
                               effective_percent);
    }

    state->safety_active = active;
    string_copy(state->safety_reason, sizeof(state->safety_reason),
                active ? reason : "");
}
