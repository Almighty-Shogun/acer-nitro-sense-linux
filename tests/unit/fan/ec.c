#include "cases.h"

#include "../fixture.h"
#include "fan/control.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Set the CPU fan through the normal fan-control path.
 *
 * The EC failure tests only target the CPU write register, so this wrapper
 * keeps each assertion focused on the state transition being verified.
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
 * Return whether the first failed EC write produced the expected fan state.
 *
 * A single write failure should be visible on the affected fan immediately,
 * but should not yet require the global safety fallback.
 */
static bool immediate_write_failure_matches(const fan_state* state, const int result)
{
    const bool result_ok = result == -1;
    const bool failure_count_ok = state->ec_write_failures == 1;
    const bool reason_ok = strcmp(state->safety_reason, "ec-write-failure") == 0;

    return result_ok && failure_count_ok && reason_ok;
}

/**
 * Return whether repeated write failures activated global EC safety.
 *
 * This validates the safety threshold separately from the individual fan state
 * so the recovery assertions can stay narrow.
 */
static bool repeated_write_failure_matches(const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    const bool threshold_ok = states[0].ec_write_failures >= cfg->safety.max_ec_write_failures;
    const bool reason_ok = strcmp(global_safety_reason(cfg, states), "ec-write-failure") == 0;

    return threshold_ok && reason_ok;
}

/**
 * Return whether the first successful write uses the critical safety speed.
 *
 * Once the fake EC stops failing writes, the daemon should clear the failure
 * counter only after it has pushed the critical recovery speed.
 */
static bool recovery_write_matches(const fan_state* state, const int result)
{
    const bool result_ok = result == 100;
    const bool failure_count_ok = state->ec_write_failures == 0;
    const bool reason_ok = strcmp(state->safety_reason, "ec-write-failure") == 0;

    return result_ok && failure_count_ok && reason_ok;
}

/**
 * Return whether the next successful write clears EC write safety.
 *
 * The recovery path intentionally requires one critical write before normal
 * fan control resumes.
 */
static bool cleared_write_failure_matches(const fan_state* state, const int result)
{
    const bool result_ok = result == 50;
    const bool reason_ok = strcmp(state->safety_reason, "") == 0;

    return result_ok && reason_ok;
}

/**
 * Verify EC write failure handling.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_fan_ec_write_failures(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS]
)
{
    int failures = 0;

    reset_unit_test_states(cfg, states);
    setenv("ANS_FAKE_EC_WRITE_FAIL_REG", "0x37", 1);

    const int immediate_result = set_cpu_fan_percent(ec, cfg, states, 50);
    const bool immediate_failure_ok = immediate_write_failure_matches(&states[0], immediate_result);

    if (!immediate_failure_ok)
    {
        fprintf(stderr, "unit-test failed: immediate EC write failure safety state\n");

        failures++;
    }

    set_cpu_fan_percent(ec, cfg, states, 50);
    set_cpu_fan_percent(ec, cfg, states, 50);

    unsetenv("ANS_FAKE_EC_WRITE_FAIL_REG");

    const bool repeated_failure_ok = repeated_write_failure_matches(cfg, states);

    if (!repeated_failure_ok)
    {
        fprintf(stderr, "unit-test failed: repeated EC write failure global safety\n");

        failures++;
    }

    const int recovery_result = set_cpu_fan_percent(ec, cfg, states, 50);
    const bool recovery_write_ok = recovery_write_matches(&states[0], recovery_result);

    if (!recovery_write_ok)
    {
        fprintf(stderr, "unit-test failed: EC write failure recovery critical write\n");

        failures++;
    }

    const int clear_result = set_cpu_fan_percent(ec, cfg, states, 50);
    const bool clear_write_ok = cleared_write_failure_matches(&states[0], clear_result);

    if (!clear_write_ok)
    {
        fprintf(stderr, "unit-test failed: EC write failure safety clear\n");

        failures++;
    }

    return failures;
}
