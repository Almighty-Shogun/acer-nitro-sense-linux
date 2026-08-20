#include "selftest/fan_cases.h"

#include "fan/control.h"
#include "selftest/fixture.h"

#include <stdio.h>
#include <string.h>

int selftest_run_fan_safety_clamps(struct ec_device *ec, struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    states[0].temp_available = false;
    states[0].temp_c = -1;
    states[0].control_temp_available = false;
    states[0].control_temp_c = -1;
    if (set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], 25, "") != 62 ||
        strcmp(states[0].safety_reason, "temperature-unknown") != 0) {
        fprintf(stderr, "self-test failed: per-fan missing temperature safety clamp\n");
        failures++;
    }

    reset_self_test_states(cfg, states);
    states[0].temp_c = 65;
    states[0].sensor_temp_c = 65;
    states[0].control_temp_c = 65;
    states[0].control_sensor_temp_c = 65;
    if (set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], 20, "") != 35 ||
        strcmp(states[0].safety_reason, "minimum-safe-speed") != 0) {
        fprintf(stderr, "self-test failed: minimum safe speed clamp\n");
        failures++;
    }

    return failures;
}
