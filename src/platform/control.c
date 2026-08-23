#include "platform/control.h"

#include <string.h>

const char FIRMWARE_AUTO_PRESET[] = "firmware-auto";

/**
 * Return whether the daemon is in firmware-auto mode.
 *
 * Platform controls mirror Acer Sense features that are actually reachable on
 * Linux. Unsupported firmware surfaces should report clearly instead of
 * pretending to work.
 */
bool firmware_auto_mode(const bool auto_mode, const char* preset)
{
    return !auto_mode && strcmp(preset, FIRMWARE_AUTO_PRESET) == 0;
}

/**
 * Resolve the active fan control mode.
 *
 * Platform controls mirror Acer Sense features that are actually reachable on
 * Linux. Unsupported firmware surfaces should report clearly instead of
 * pretending to work.
 */
const char* control_mode(const bool auto_mode, const char* preset)
{
    if (auto_mode)
        return "auto";

    if (firmware_auto_mode(auto_mode, preset))
        return FIRMWARE_AUTO_PRESET;

    if (strcmp(preset, "manual") == 0)
        return "manual";

    return "preset";
}
