#include "selftest/fan_cases.h"

#include "fan/control.h"
#include "selftest/fixture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void clear_seeded_temperatures(const struct ans_config *cfg,
                                      fan_state states[ANS_MAX_FANS])
{
    for (int i = 0; i < cfg->fan_len; i++) {
        states[i].temp_c = -1;
        states[i].control_temp_c = -1;
        states[i].temp_available = false;
        states[i].control_temp_available = false;
        states[i].temp_seeded = false;
        states[i].control_temp_seeded = false;
    }
}

int selftest_run_fan_temperature_safety(struct ec_device *ec,
                                        struct ans_config *cfg,
                                        fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    reset_self_test_states(cfg, states);
    states[0].percent = 25;
    states[1].percent = 25;
    states[0].requested_percent = 25;
    states[1].requested_percent = 25;
    setenv("ANS_FAKE_CPU_TEMP_C", "91", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "50", 1);
    update_fan_states(ec, cfg, states, false, "manual");
    if (states[0].percent != 25 || states[1].percent != 25 ||
        states[0].temp_c != 45 || states[0].sensor_temp_c != 91 ||
        states[0].pending_spike_temp_c != 91 ||
        strcmp(states[0].safety_reason, "") != 0) {
        fprintf(stderr, "self-test failed: one-sample temperature spike filter\n");
        failures++;
    }

    update_fan_states(ec, cfg, states, false, "manual");
    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");
    if (states[0].percent != 100 || states[1].percent != 100 ||
        strcmp(states[0].safety_reason, "critical-temperature") != 0 ||
        strcmp(states[1].safety_reason, "critical-temperature") != 0) {
        fprintf(stderr, "self-test failed: global critical safety override\n");
        failures++;
    }

    setenv("ANS_FAKE_CPU_TEMP_C", "55", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "50", 1);
    update_fan_states(ec, cfg, states, false, "manual");
    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");
    if (states[0].percent != 25 || states[1].percent != 25 ||
        strcmp(states[0].safety_reason, "") != 0 ||
        strcmp(states[1].safety_reason, "") != 0) {
        fprintf(stderr, "self-test failed: critical safety restores requested speed\n");
        failures++;
    }

    reset_self_test_states(cfg, states);
    clear_seeded_temperatures(cfg, states);
    setenv("ANS_FAKE_CPU_TEMP_C", "93", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "60", 1);
    update_fan_states(ec, cfg, states, true, "auto");
    if (states[0].percent == 100 || states[1].percent == 100 ||
        strcmp(states[0].safety_reason, "critical-temperature") == 0 ||
        strcmp(states[1].safety_reason, "critical-temperature") == 0) {
        fprintf(stderr, "self-test failed: unseeded startup critical hold\n");
        failures++;
    }

    update_fan_states(ec, cfg, states, true, "auto");
    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");
    if (states[0].percent != 100 || states[1].percent != 100 ||
        strcmp(states[0].safety_reason, "critical-temperature") != 0 ||
        strcmp(states[1].safety_reason, "critical-temperature") != 0) {
        fprintf(stderr, "self-test failed: repeated startup critical accepted\n");
        failures++;
    }

    return failures;
}

int selftest_run_fan_ec_temperature_register(struct ec_device *ec,
                                             struct ans_config *cfg,
                                             fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    reset_self_test_states(cfg, states);
    clear_seeded_temperatures(cfg, states);
    cfg->fans[1].temperature_register = 0x2a;
    ec->fake_regs[0x2a] = 63;
    setenv("ANS_FAKE_CPU_TEMP_C", "58", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "45", 1);

    update_fan_states(ec, cfg, states, false, "manual");

    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");
    cfg->fans[1].temperature_register = -1;

    if (!states[1].temp_available || states[1].sensor_temp_c != 63 ||
        states[1].temp_c != 63) {
        fprintf(stderr, "self-test failed: EC GPU temperature register fallback\n");
        failures++;
    }

    return failures;
}
