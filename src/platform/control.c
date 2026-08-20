#include "platform/control.h"

#include <string.h>

const char FIRMWARE_AUTO_PRESET[] = "firmware-auto";

bool firmware_auto_mode(const bool auto_mode, const char *preset)
{
    return !auto_mode && strcmp(preset, FIRMWARE_AUTO_PRESET) == 0;
}

const char *control_mode(const bool auto_mode, const char *preset)
{
    if (auto_mode)
        return "auto";
    if (firmware_auto_mode(auto_mode, preset))
        return FIRMWARE_AUTO_PRESET;
    if (strcmp(preset, "manual") == 0)
        return "manual";
    return "preset";
}
