#ifndef ANS_CONFIG_CONFIG_H
#define ANS_CONFIG_CONFIG_H

#include "config/types.h"

/**
 * Load and validate a model profile from JSON.
 *
 * The loader fills defaults, parses every supported section, and rejects
 * profiles that would produce unsafe or incomplete fan control.
 */
int config_load(const char* path, struct ans_config* cfg);

/**
 * Find a named fan preset in a loaded profile.
 *
 * Preset lookup is case-sensitive because preset identifiers are part of the
 * command protocol and persisted state.
 */
const struct preset_config* config_find_preset(const struct ans_config* cfg, const char* id);

/**
 * Find a named platform profile in a loaded profile.
 *
 * The result provides the EC value used for firmware profile writes.
 */
const struct platform_profile_entry* config_find_platform_profile(const struct ans_config* cfg, const char* id);

/**
 * Find a configured fan by id.
 *
 * Manual fan commands use this to resolve user-facing ids such as cpu and gpu
 * to model-specific EC registers.
 */
const struct fan_config* config_find_fan(const struct ans_config* cfg, const char* id);

#endif
