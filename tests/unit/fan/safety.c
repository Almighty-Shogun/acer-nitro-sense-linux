#include "cases.h"

#include "../fixture.h"
#include "fan/control.h"

#include <stdio.h>
#include <string.h>

/**
 * Set the CPU fan through the normal fan-control path.
 *
 * Safety clamps must be checked through the public fan-control function,
 * because that is where requested percentages are converted into safe writes.
 */
static int set_cpu_fan_percent(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    const int percent
)
{
    return set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], percent, "");
}

/**
 * Return whether a missing temperature forced the configured fallback speed.
 *
 * A fan without a usable temperature sensor must not honor a low manual value,
 * because the daemon cannot prove the component is safe.
 */
static bool missing_temperature_clamp_matches(const fan_state* state, const int result)
{
    const bool speed_ok = result == 62;
    const bool reason_ok = strcmp(state->safety_reason, "temperature-unknown") == 0;

    return speed_ok && reason_ok;
}

/**
 * Return whether a hot component forced the minimum safe speed.
 *
 * This protects the boundary where manual requests below the safe floor are
 * raised while the component is already warm.
 */
static bool minimum_safe_speed_clamp_matches(const fan_state* state, const int result)
{
    const bool speed_ok = result == 35;
    const bool reason_ok = strcmp(state->safety_reason, "minimum-safe-speed") == 0;

    return speed_ok && reason_ok;
}

/**
 * Verify fan safety clamps.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_fan_safety_clamps(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    states[0].temp_available = false;
    states[0].temp_c = -1;
    states[0].control_temp_available = false;
    states[0].control_temp_c = -1;

    const int missing_temperature_result = set_cpu_fan_percent(ec, cfg, states, 25);
    const bool missing_temperature_ok = missing_temperature_clamp_matches(&states[0], missing_temperature_result);

    if (!missing_temperature_ok)
    {
        fprintf(stderr, "unit-test failed: per-fan missing temperature safety clamp\n");

        failures++;
    }

    reset_unit_test_states(cfg, states);

    states[0].temp_c = 65;
    states[0].sensor_temp_c = 65;
    states[0].control_temp_c = 65;
    states[0].control_sensor_temp_c = 65;

    const int minimum_safe_speed_result = set_cpu_fan_percent(ec, cfg, states, 20);
    const bool minimum_safe_speed_ok = minimum_safe_speed_clamp_matches(&states[0], minimum_safe_speed_result);

    if (!minimum_safe_speed_ok)
    {
        fprintf(stderr, "unit-test failed: minimum safe speed clamp\n");

        failures++;
    }

    return failures;
}
