#ifndef ANS_DAEMON_UNIT_FAN_CASES_H
#define ANS_DAEMON_UNIT_FAN_CASES_H

#include "daemon/types.h"

/**
 * Run fan safety clamp tests.
 *
 * These cases verify that low manual requests are raised when safety policy
 * requires a higher effective speed.
 */
int unit_run_fan_safety_clamps(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

/**
 * Run fan EC write failure tests.
 *
 * These cases verify failure counters and safety fallbacks when EC writes do
 * not succeed.
 */
int unit_run_fan_ec_write_failures(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

/**
 * Run fan temperature safety tests.
 *
 * These cases cover critical-temperature escalation and release behavior.
 */
int unit_run_fan_temperature_safety(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

/**
 * Run EC temperature register tests.
 *
 * These cases verify model profiles that read temperature directly from EC
 * registers.
 */
int unit_run_fan_ec_temperature_register(struct ec_device* ec, struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

/**
 * Run automatic ramping tests.
 *
 * These cases verify that daemon-auto avoids noisy jumps unless safety policy
 * bypasses the ramp.
 */
int unit_run_fan_auto_ramp(struct ec_device* ec, struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

/**
 * Run preset resume status tests.
 *
 * These cases verify that restored preset state is reflected in status output
 * after resume.
 */
int unit_run_fan_preset_resume_status(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

#endif
