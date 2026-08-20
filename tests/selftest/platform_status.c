#include "selftest/platform_cases.h"

#include "selftest/helpers.h"

#include <stdio.h>
#include <string.h>

int selftest_run_platform_status_commands(struct ec_device *ec,
                                          const struct ans_config *cfg,
                                          fan_state states[ANS_MAX_FANS],
                                          bool *auto_mode, char *preset,
                                          const size_t preset_len,
                                          bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    if (selftest_execute_command("status\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 false, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "model=Acer Nitro Self Test mode=manual") ||
        !strstr(reply, "coolboost=off")) {
        fprintf(stderr, "self-test failed: status command path\n");
        failures++;
    }

    if (selftest_execute_command("coolboost status\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "coolboost=off")) {
        fprintf(stderr, "self-test failed: coolboost status command path\n");
        failures++;
    }

    if (selftest_execute_command("capabilities\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "fan_control=available") ||
        !strstr(reply, "fan_mode=available modes=auto,manual,turbo") ||
        !strstr(reply, "platform_profile=available profiles=quiet,balanced,performance") ||
        !strstr(reply, "power_source_profile=available auto_apply=off")) {
        fprintf(stderr, "self-test failed: capabilities command path\n");
        failures++;
    }

    return failures;
}
