#include "selftest/fan_cases.h"

#include "daemon/status.h"
#include "ec/ec.h"
#include "fan/control.h"
#include "selftest/fixture.h"
#include "selftest/helpers.h"

#include <stdio.h>
#include <string.h>

int selftest_run_fan_preset_resume_status(struct ec_device *ec,
                                          struct ans_config *cfg,
                                          fan_state states[ANS_MAX_FANS])
{
    int failures = 0;
    char reply[1024];

    reset_self_test_states(cfg, states);
    if (!apply_preset(ec, cfg, states, "balanced") ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 50 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 45) {
        fprintf(stderr, "self-test failed: fake EC preset writes\n");
        failures++;
    }

    states[0].percent = 70;
    states[1].percent = 65;
    states[0].requested_percent = 70;
    states[1].requested_percent = 65;
    apply_current_control_state(ec, cfg, states);
    if (ec_read_byte(ec, cfg->fans[0].write_register) != 70 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 65) {
        fprintf(stderr, "self-test failed: resume reapply writes\n");
        failures++;
    }

    if (selftest_read_reply(reply_preset_show, cfg, states, false, "manual",
                            reply, sizeof(reply)) < 0 ||
        !strstr(reply, "mode=manual preset=manual") ||
        !strstr(reply, "cpu requested=70 effective=70 percent=70")) {
        fprintf(stderr, "preset show reply:\n%s", reply);
        fprintf(stderr, "self-test failed: preset show response format\n");
        failures++;
    }

    return failures;
}
