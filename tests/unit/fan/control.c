#include "control.h"

#include "cases.h"

/**
 * Run fan control tests.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_fan_control(struct ec_device* ec, struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    failures += unit_run_fan_safety_clamps(ec, cfg, states);
    failures += unit_run_fan_ec_write_failures(ec, cfg, states);
    failures += unit_run_fan_temperature_safety(ec, cfg, states);
    failures += unit_run_fan_ec_temperature_register(ec, cfg, states);
    failures += unit_run_fan_auto_ramp(ec, cfg, states);
    failures += unit_run_fan_preset_resume_status(ec, cfg, states);

    return failures;
}
