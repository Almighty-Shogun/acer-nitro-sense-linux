#ifndef ANS_DAEMON_STATUS_FORMAT_H
#define ANS_DAEMON_STATUS_FORMAT_H

#include <stdbool.h>

static inline const char *status_fan_control_source(const bool firmware_mode,
                                                    const bool safety_active)
{
    if (safety_active)
        return "safety";

    return firmware_mode ? "firmware" : "daemon";
}

#endif
