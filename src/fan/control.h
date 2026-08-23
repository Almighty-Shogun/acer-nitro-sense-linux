#ifndef ANS_FAN_CONTROL_H
#define ANS_FAN_CONTROL_H

#include "daemon/types.h"

/**
 * Apply a requested fan percentage with safety policy.
 *
 * The requested value is adjusted for safety, converted to the model's EC
 * write range, and written to the fan register.
 */
int set_fan_percent(
    struct ec_device* ec,
    const struct ans_config* cfg,
    const struct fan_config* fan,
    fan_state* state,
    int requested_percent,
    const char* forced_reason
);

/**
 * Find the global fan safety reason.
 *
 * A critical condition on one component can force safer speeds on every fan,
 * so callers need the highest-priority active reason.
 */
const char* global_safety_reason(const struct ans_config* cfg, const fan_state states[ANS_MAX_FANS]);

/**
 * Apply model-specific EC initialization writes.
 *
 * Init writes prepare firmware state before the daemon starts reading sensors
 * or writing fan speeds.
 */
void apply_init_writes(struct ec_device* ec, const struct ans_config* cfg);

/**
 * Apply model-specific EC reset writes.
 *
 * Reset writes return firmware state to the profile's expected defaults during
 * shutdown.
 */
void apply_reset_writes(struct ec_device* ec, const struct ans_config* cfg);

/**
 * Limit automatic fan-speed changes to a configured ramp.
 *
 * Gradual ramping prevents normal automatic control from jumping to a much
 * louder speed unless safety policy requires it.
 */
int auto_ramped_percent(const struct ans_config* cfg, const fan_state* state, int target_percent, const char* forced_reason);

/**
 * Reconcile fan targets with sensors and safety policy.
 *
 * This is the main cooling reconciliation step: it samples temperatures,
 * applies curve or preset intent, and lets safety policy override unsafe
 * requests before EC writes.
 */
void update_fan_states(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset
);

/**
 * Seed fan temperature state before the polling loop runs.
 *
 * Startup seeding gives the spike filter a trusted baseline before automatic
 * control decisions begin.
 */
void seed_last_temperatures(const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

/**
 * Set one fan target or every fan target.
 *
 * Manual fan commands share this path so cpu, gpu, and all targets receive
 * the same safety reason and write handling.
 */
int set_one(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS], const char* id, int percent);

/**
 * Apply a named fan preset from the model configuration.
 *
 * Named presets are model configuration, not hard-coded daemon behavior.
 * Looking them up here keeps command handling independent of profile
 * contents.
 */
bool apply_preset(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS], const char* id);

/**
 * Reapply the saved fan control state.
 *
 * Resume paths call this after restoring state so the EC reflects the daemon
 * state that users see in status output.
 */
void apply_current_control_state(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

#endif
