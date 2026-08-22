#include "cases.h"

#include "fan/control.h"
#include "../fixture.h"

#include <stdio.h>
#include <stdlib.h>

int unit_run_fan_auto_ramp(struct ec_device *ec, struct ans_config *cfg,
                               fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    reset_unit_test_states(cfg, states);
    states[0].percent = 25;
    states[1].percent = 25;
    states[0].requested_percent = 25;
    states[1].requested_percent = 25;
    setenv("ANS_FAKE_CPU_TEMP_C", "65", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "45", 1);
    update_fan_states(ec, cfg, states, true, "auto");
    update_fan_states(ec, cfg, states, true, "auto");
    if (states[0].percent != 42 || states[1].percent != 37 ||
        states[1].temp_c != 45 || states[1].control_temp_c != 65) {
        fprintf(stderr, "unit-test failed: auto ramped shared max control temperature\n");
        failures++;
    }

    update_fan_states(ec, cfg, states, true, "auto");
    update_fan_states(ec, cfg, states, true, "auto");
    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");
    if (states[0].percent != 55 || states[1].percent != 55) {
        fprintf(stderr, "unit-test failed: auto ramp reaches curve target\n");
        failures++;
    }

    states[0].requested_percent = 50;
    states[0].percent = 50;
    states[0].control_temp_available = true;
    states[0].control_temp_c = cfg->critical_temperature_c;
    if (auto_ramped_percent(cfg, &states[0], 100, "") != 62 ||
        auto_ramped_percent(cfg, &states[0], 80, "") != 62) {
        fprintf(stderr, "unit-test failed: auto ramp limit without temperature bypass\n");
        failures++;
    }

    if (auto_ramped_percent(cfg, &states[0], 100, "critical-temperature") != 100) {
        fprintf(stderr, "unit-test failed: auto ramp forced bypass\n");
        failures++;
    }

    cfg->safety.auto_ramp_bypass_temperature_c = 88;
    states[0].control_temp_c = cfg->safety.auto_ramp_bypass_temperature_c;
    if (auto_ramped_percent(cfg, &states[0], 100, "") != 100) {
        fprintf(stderr, "unit-test failed: configured auto ramp temperature bypass\n");
        failures++;
    }
    cfg->safety.auto_ramp_bypass_temperature_c = 0;

    return failures;
}
