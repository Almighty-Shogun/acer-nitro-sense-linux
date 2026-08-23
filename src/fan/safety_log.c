#include "fan/internal.h"

#include "util/string.h"

#include <stdio.h>
#include <string.h>

#define FAN_SAFETY_ACTIVE_LOG_FORMAT \
    "safety active fan=%s reason=%s requested=%d effective=%d " \
    "temp=%d control_temp=%d rpm=%d " \
    "ec_read_failures=%d ec_write_failures=%d\n"

#define FAN_SAFETY_CLEARED_LOG_FORMAT \
    "safety cleared fan=%s previous=%s requested=%d effective=%d " \
    "temp=%d control_temp=%d rpm=%d\n"

/**
 * Return whether a safety reason string is present.
 *
 * Empty strings represent normal cooling. A non-empty value means safety
 * policy is overriding the requested fan behavior.
 */
static bool text_has_value(const char* text)
{
    return text && text[0] != '\0';
}

/**
 * Log that safety policy started overriding a fan.
 *
 * The log includes both requested and effective percentages so fan noise can be
 * traced back to either user intent or thermal protection.
 */
static void log_safety_active(
    const struct fan_config* fan,
    const fan_state* state,
    const char* reason,
    const int requested_percent,
    const int effective_percent
)
{
    if (daemon_quiet_logs) return;

    fprintf(
        stderr,
        FAN_SAFETY_ACTIVE_LOG_FORMAT,
        fan->id,
        reason,
        requested_percent,
        effective_percent,
        state->temp_c,
        state->control_temp_c,
        state->rpm,
        state->ec_read_failures,
        state->ec_write_failures
    );
}

/**
 * Log that safety policy stopped overriding a fan.
 *
 * The previous safety reason is emitted before the state is cleared so logs can
 * show what condition recovered.
 */
static void log_safety_cleared(
    const struct fan_config* fan,
    const fan_state* state,
    const int requested_percent,
    const int effective_percent
)
{
    if (daemon_quiet_logs) return;

    fprintf(
        stderr,
        FAN_SAFETY_CLEARED_LOG_FORMAT,
        fan->id,
        state->safety_reason,
        requested_percent,
        effective_percent,
        state->temp_c,
        state->control_temp_c,
        state->rpm
    );
}

/**
 * Update the fan safety state and emit transition logs.
 *
 * Logs are emitted only when safety state changes, avoiding repeated messages
 * during steady high-temperature or EC-failure conditions.
 */
void fan_update_safety_state(
    const struct fan_config* fan,
    fan_state* state,
    const char* reason,
    const int requested_percent,
    const int effective_percent
)
{
    const bool active = text_has_value(reason);
    const bool changed = active != state->safety_active || (active && strcmp(state->safety_reason, reason) != 0);

    if (changed)
    {
        if (active)
        {
            log_safety_active(fan, state, reason, requested_percent, effective_percent);
        }
        else
        {
            log_safety_cleared(fan, state, requested_percent, effective_percent);
        }
    }

    state->safety_active = active;

    string_copy(state->safety_reason, sizeof(state->safety_reason), active ? reason : "");
}
