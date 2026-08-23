#ifndef ANS_HARDWARE_H
#define ANS_HARDWARE_H

#include <stdbool.h>

struct ans_config;
struct fan_config;

/**
 * Human-readable hardware names used in status output.
 *
 * Names are discovered from Linux hardware metadata and mapped back to the
 * configured CPU and GPU fan components.
 */
typedef struct
{
    char cpu[160];
    char gpu[160];
} hardware_names;

/**
 * Load the system model name from DMI.
 *
 * The result is used to protect users from accidentally applying a model
 * profile to unrelated hardware.
 */
const char* load_dmi_model(void);

/**
 * Return whether the current DMI model is allowed by a model profile.
 *
 * Empty allow-lists are treated as unrestricted so development profiles can be
 * validated before their DMI names are finalized.
 */
bool dmi_allowed(const struct ans_config* cfg, const char* dmi);

/**
 * Load display names for CPU and GPU hardware.
 *
 * Status output prefers real component names when Linux exposes them and falls
 * back to profile labels otherwise.
 */
void load_hardware_names(hardware_names* names);

/**
 * Resolve the display name for a fan component.
 *
 * The mapping uses the fan's sensor group so CPU and GPU entries receive the
 * right discovered hardware name.
 */
const char* component_name_for_fan(const hardware_names* names, const struct fan_config* fan);

#endif
