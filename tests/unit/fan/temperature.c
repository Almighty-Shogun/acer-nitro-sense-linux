#include "cases.h"

#include "../fixture.h"
#include "fan/control.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Clear seeded fan temperatures.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static void clear_seeded_temperatures(const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    for (int i = 0; i < cfg->fan_len; i++)
    {
        states[i].temp_c = -1;
        states[i].control_temp_c = -1;
        states[i].temp_available = false;
        states[i].control_temp_available = false;
        states[i].temp_seeded = false;
        states[i].control_temp_seeded = false;
    }
}

/**
 * Seed both fans with the same requested manual speed.
 *
 * Temperature safety tests need a low requested value so critical overrides and
 * recovery can prove they are changing the effective fan speed.
 */
static void seed_requested_percent(fan_state states[ANS_MAX_FANS], const int percent)
{
    states[0].percent = percent;
    states[1].percent = percent;
    states[0].requested_percent = percent;
    states[1].requested_percent = percent;
}

/**
 * Set fake CPU and GPU temperatures for the next fan-state update.
 *
 * The daemon reads temperatures from injected environment values in these unit
 * tests, which keeps the safety logic independent from host hardware.
 */
static void set_fake_temperatures(const char* cpu_temp, const char* gpu_temp)
{
    setenv("ANS_FAKE_CPU_TEMP_C", cpu_temp, 1);
    setenv("ANS_FAKE_GPU_TEMP_C", gpu_temp, 1);
}

/**
 * Clear fake CPU and GPU temperatures.
 *
 * Tests call this immediately after the update that consumes the injected
 * values so later assertions cannot inherit stale temperature inputs.
 */
static void clear_fake_temperatures(void)
{
    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");
}

/**
 * Return whether the first hot sample was treated as a spike.
 *
 * One isolated CPU temperature spike should be remembered as pending data while
 * the filtered temperature and requested fan speeds stay unchanged.
 */
static bool one_sample_spike_matches(fan_state states[ANS_MAX_FANS])
{
    const bool fan_speed_ok = states[0].percent == 25 && states[1].percent == 25;
    const bool filtered_temp_ok = states[0].temp_c == 45;
    const bool sensor_temp_ok = states[0].sensor_temp_c == 91;
    const bool pending_spike_ok = states[0].pending_spike_temp_c == 91;
    const bool safety_ok = strcmp(states[0].safety_reason, "") == 0;

    return fan_speed_ok && filtered_temp_ok && sensor_temp_ok && pending_spike_ok && safety_ok;
}

/**
 * Return whether global critical temperature safety is active.
 *
 * A confirmed critical CPU temperature should force both fans to full speed,
 * because the platform shares thermal headroom across CPU and GPU cooling.
 */
static bool global_critical_safety_matches(fan_state states[ANS_MAX_FANS])
{
    const bool fan_speed_ok = states[0].percent == 100 && states[1].percent == 100;

    const bool cpu_reason_ok = strcmp(states[0].safety_reason, "critical-temperature") == 0;
    const bool gpu_reason_ok = strcmp(states[1].safety_reason, "critical-temperature") == 0;

    return fan_speed_ok && cpu_reason_ok && gpu_reason_ok;
}

/**
 * Return whether critical safety restored the requested speed.
 *
 * Once temperatures fall back into the safe range, the daemon should return to
 * the previously requested fan speed and clear both safety reasons.
 */
static bool critical_safety_cleared_matches(fan_state states[ANS_MAX_FANS])
{
    const bool fan_speed_ok = states[0].percent == 25 && states[1].percent == 25;

    const bool cpu_reason_ok = strcmp(states[0].safety_reason, "") == 0;
    const bool gpu_reason_ok = strcmp(states[1].safety_reason, "") == 0;

    return fan_speed_ok && cpu_reason_ok && gpu_reason_ok;
}

/**
 * Return whether startup critical safety is still being held.
 *
 * Unseeded startup samples get one update to prove they are stable before the
 * daemon accepts them as critical temperatures.
 */
static bool startup_critical_hold_matches(fan_state states[ANS_MAX_FANS])
{
    const bool fan_speed_ok = states[0].percent != 100 && states[1].percent != 100;

    const bool cpu_reason_ok = strcmp(states[0].safety_reason, "critical-temperature") != 0;
    const bool gpu_reason_ok = strcmp(states[1].safety_reason, "critical-temperature") != 0;

    return fan_speed_ok && cpu_reason_ok && gpu_reason_ok;
}

/**
 * Return whether the GPU EC temperature register was used as a fallback.
 *
 * This protects systems where the normal GPU sensor path is unavailable but
 * the model profile provides an EC temperature register.
 */
static bool gpu_ec_temperature_fallback_matches(fan_state states[ANS_MAX_FANS])
{
    const bool available_ok = states[1].temp_available;
    const bool sensor_temp_ok = states[1].sensor_temp_c == 63;
    const bool filtered_temp_ok = states[1].temp_c == 63;

    return available_ok && sensor_temp_ok && filtered_temp_ok;
}

/**
 * Verify temperature safety behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_fan_temperature_safety(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    reset_unit_test_states(cfg, states);

    seed_requested_percent(states, 25);

    set_fake_temperatures("91", "50");

    update_fan_states(ec, cfg, states, false, "manual");

    const bool one_sample_spike_ok = one_sample_spike_matches(states);

    if (!one_sample_spike_ok)
    {
        fprintf(stderr, "unit-test failed: one-sample temperature spike filter\n");

        failures++;
    }

    update_fan_states(ec, cfg, states, false, "manual");

    clear_fake_temperatures();

    const bool global_critical_ok = global_critical_safety_matches(states);

    if (!global_critical_ok)
    {
        fprintf(stderr, "unit-test failed: global critical safety override\n");

        failures++;
    }

    set_fake_temperatures("55", "50");

    update_fan_states(ec, cfg, states, false, "manual");

    clear_fake_temperatures();

    const bool critical_clear_ok = critical_safety_cleared_matches(states);

    if (!critical_clear_ok)
    {
        fprintf(stderr, "unit-test failed: critical safety restores requested speed\n");
        failures++;
    }

    reset_unit_test_states(cfg, states);
    clear_seeded_temperatures(cfg, states);

    set_fake_temperatures("93", "60");

    update_fan_states(ec, cfg, states, true, "auto");

    const bool startup_hold_ok = startup_critical_hold_matches(states);

    if (!startup_hold_ok)
    {
        fprintf(stderr, "unit-test failed: unseeded startup critical hold\n");

        failures++;
    }

    update_fan_states(ec, cfg, states, true, "auto");

    clear_fake_temperatures();

    const bool repeated_critical_ok = global_critical_safety_matches(states);

    if (!repeated_critical_ok)
    {
        fprintf(stderr, "unit-test failed: repeated startup critical accepted\n");

        failures++;
    }

    return failures;
}

/**
 * Verify EC temperature register fallback.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_fan_ec_temperature_register(struct ec_device* ec, struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    reset_unit_test_states(cfg, states);
    clear_seeded_temperatures(cfg, states);

    cfg->fans[1].temperature_register = 0x2a;
    ec->fake_regs[0x2a] = 63;

    set_fake_temperatures("58", "45");

    update_fan_states(ec, cfg, states, false, "manual");

    clear_fake_temperatures();

    cfg->fans[1].temperature_register = -1;

    const bool gpu_temperature_fallback_ok = gpu_ec_temperature_fallback_matches(states);

    if (!gpu_temperature_fallback_ok)
    {
        fprintf(stderr, "unit-test failed: EC GPU temperature register fallback\n");

        failures++;
    }

    return failures;
}
