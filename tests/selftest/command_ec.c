#include "selftest/command_cases.h"

#include "selftest/helpers.h"

#include <stdio.h>
#include <string.h>

int selftest_run_ec_debug_commands(struct ec_device *ec,
                                   const struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS],
                                   bool *auto_mode, char *preset,
                                   const size_t preset_len,
                                   bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    ec_write_byte(ec, 0x40, 0xab);
    ec_write_byte(ec, 0x41, 0xcd);
    if (selftest_execute_command("ec-read 0x40\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "ec[0x40]=0xab (171)")) {
        fprintf(stderr, "self-test failed: ec read command path\n");
        failures++;
    }

    if (selftest_execute_command("ec-dump 0x40 0x41\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "0x40: ab cd")) {
        fprintf(stderr, "self-test failed: ec dump command path\n");
        failures++;
    }

    if (selftest_execute_command("ec-read 0x40\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "error permission denied")) {
        fprintf(stderr, "self-test failed: denied ec read command path\n");
        failures++;
    }

    return failures;
}
