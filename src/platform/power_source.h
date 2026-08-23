#ifndef ANS_POWER_SOURCE_H
#define ANS_POWER_SOURCE_H

#include <stdbool.h>

struct ans_config;
struct ec_device;

/**
 * Power source detected from Linux power-supply state.
 *
 * The daemon uses this normalized value to choose AC or battery firmware
 * profiles without exposing sysfs naming differences.
 */
enum power_source_state
{
    POWER_SOURCE_UNKNOWN = 0,
    POWER_SOURCE_AC,
    POWER_SOURCE_BATTERY,
};

/**
 * Format the current power source name.
 *
 * The returned strings are used directly in daemon status and command replies.
 */
const char* power_source_name(enum power_source_state source);

/**
 * Read the active AC or battery power source.
 *
 * Mains power takes precedence when any online AC supply is present.
 */
enum power_source_state read_power_source(void);

/**
 * Select the profile for the active power source.
 *
 * Unknown sources return NULL so callers do not apply a profile without a
 * reliable power-state signal.
 */
const char* power_source_profile_for(const struct ans_config* cfg, enum power_source_state source);

/**
 * Return whether AC and battery profiles are fully configured.
 *
 * Auto-apply is useful only when both sides of the policy are known.
 */
bool power_source_profile_policy_available(const struct ans_config* cfg);

/**
 * Apply the configured firmware profile for the current power source.
 *
 * The platform profile backend owns the actual EC write; this helper chooses
 * which profile should be written.
 */
bool apply_power_source_profile(struct ec_device* ec, const struct ans_config* cfg, enum power_source_state source);

#endif
