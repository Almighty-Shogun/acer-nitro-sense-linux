#include "selftest/fan_cases.h"

#include "fan/control.h"
#include "selftest/fixture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int selftest_run_fan_ec_write_failures(struct ec_device *ec,
                                       struct ans_config *cfg,
                                       fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    reset_self_test_states(cfg, states);
    setenv("ANS_FAKE_EC_WRITE_FAIL_REG", "0x37", 1);
    if (set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], 50, "") != -1 ||
        states[0].ec_write_failures != 1 ||
        strcmp(states[0].safety_reason, "ec-write-failure") != 0) {
        fprintf(stderr, "self-test failed: immediate EC write failure safety state\n");
        failures++;
    }
    set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], 50, "");
    set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], 50, "");
    unsetenv("ANS_FAKE_EC_WRITE_FAIL_REG");
    if (states[0].ec_write_failures < cfg->safety.max_ec_write_failures ||
        strcmp(global_safety_reason(cfg, states), "ec-write-failure") != 0) {
        fprintf(stderr, "self-test failed: repeated EC write failure global safety\n");
        failures++;
    }
    if (set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], 50, "") != 100 ||
        states[0].ec_write_failures != 0 ||
        strcmp(states[0].safety_reason, "ec-write-failure") != 0) {
        fprintf(stderr, "self-test failed: EC write failure recovery critical write\n");
        failures++;
    }
    if (set_fan_percent(ec, cfg, &cfg->fans[0], &states[0], 50, "") != 50 ||
        strcmp(states[0].safety_reason, "") != 0) {
        fprintf(stderr, "self-test failed: EC write failure safety clear\n");
        failures++;
    }

    return failures;
}
