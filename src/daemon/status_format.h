#ifndef ANS_DAEMON_STATUS_FORMAT_H
#define ANS_DAEMON_STATUS_FORMAT_H

#include "daemon/types.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
#define ANS_STATUS_FORMAT_UNUSED __attribute__((unused))
#else
#define ANS_STATUS_FORMAT_UNUSED
#endif

/**
 * Return whether a fan is firmware controlled.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
static ANS_STATUS_FORMAT_UNUSED bool status_fan_firmware_controlled(const bool firmware_mode, const bool safety_active)
{
    return firmware_mode && !safety_active;
}

/**
 * Describe the visible fan control source.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
static ANS_STATUS_FORMAT_UNUSED const char* status_fan_control_source(const bool firmware_mode, const bool safety_active)
{
    if (safety_active)
        return "safety";

    return firmware_mode ? "firmware" : "daemon";
}

/**
 * Compute requested percent.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
static ANS_STATUS_FORMAT_UNUSED int status_requested_percent(const fan_state* state)
{
    return state->requested_percent > 0 ? state->requested_percent : state->percent;
}

/**
 * Compute active percent.
 *
 * Status replies are the daemon-facing representation of current hardware and
 * control state. They keep human CLI output and machine-readable state in
 * sync.
 */
static ANS_STATUS_FORMAT_UNUSED void status_active_percent_text(
    char* out,
    const size_t out_len,
    const bool firmware_mode,
    const bool safety_active,
    const int percent,
    const char* firmware_text
)
{
    if (status_fan_firmware_controlled(firmware_mode, safety_active))
    {
        snprintf(out, out_len, "%s", firmware_text);
    }
    else
    {
        snprintf(out, out_len, "%d", percent);
    }
}

#undef ANS_STATUS_FORMAT_UNUSED

#endif
