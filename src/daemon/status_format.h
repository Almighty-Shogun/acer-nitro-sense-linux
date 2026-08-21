#ifndef ANS_DAEMON_STATUS_FORMAT_H
#define ANS_DAEMON_STATUS_FORMAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static inline bool status_fan_firmware_controlled(const bool firmware_mode,
                                                  const bool safety_active)
{
    return firmware_mode && !safety_active;
}

static inline const char *status_fan_control_source(const bool firmware_mode,
                                                    const bool safety_active)
{
    if (safety_active)
        return "safety";

    return firmware_mode ? "firmware" : "daemon";
}

static inline void status_active_percent_text(char *out, const size_t out_len,
                                              const bool firmware_mode,
                                              const bool safety_active,
                                              const int percent,
                                              const char *firmware_text)
{
    if (status_fan_firmware_controlled(firmware_mode, safety_active))
        snprintf(out, out_len, "%s", firmware_text);
    else
        snprintf(out, out_len, "%d", percent);
}

#endif
