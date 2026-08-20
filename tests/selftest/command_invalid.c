#include "selftest/command_cases.h"

#include "fan/control.h"
#include "selftest/helpers.h"

#include <stdio.h>
#include <string.h>

int selftest_run_invalid_and_reset_commands(struct ec_device *ec,
                                            const struct ans_config *cfg,
                                            fan_state states[ANS_MAX_FANS],
                                            bool *auto_mode, char *preset,
                                            const size_t preset_len,
                                            bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    if (selftest_execute_command("preset nope\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "error unknown preset")) {
        fprintf(stderr, "self-test failed: unknown preset command path\n");
        failures++;
    }

    if (selftest_execute_command("set gpu 101\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "error usage: set cpu|gpu|all 1-100")) {
        fprintf(stderr, "self-test failed: malformed set command path\n");
        failures++;
    }

    if (selftest_execute_command("status extra\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "error unknown command")) {
        fprintf(stderr, "self-test failed: strict status command path\n");
        failures++;
    }

    states[0].percent = 74;
    states[1].percent = 68;
    apply_current_control_state(ec, cfg, states);
    apply_reset_writes(ec, cfg);
    if (ec_read_byte(ec, cfg->fans[0].write_register) != cfg->fans[0].reset_speed ||
        ec_read_byte(ec, cfg->fans[1].write_register) != cfg->fans[1].reset_speed) {
        fprintf(stderr, "self-test failed: reset writes\n");
        failures++;
    }

    return failures;
}
