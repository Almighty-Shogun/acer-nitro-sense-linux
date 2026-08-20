#include "selftest/platform_cases.h"

#include "ec/ec.h"
#include "selftest/helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int selftest_run_platform_power_source_commands(struct ec_device *ec,
                                                const struct ans_config *cfg,
                                                fan_state states[ANS_MAX_FANS],
                                                bool *auto_mode, char *preset,
                                                const size_t preset_len,
                                                bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    setenv("ANS_FAKE_POWER_SOURCE", "battery", 1);
    if (selftest_execute_command("power-source status\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "power_source=battery") ||
        !strstr(reply, "target_profile=quiet")) {
        fprintf(stderr, "self-test failed: power-source status command path\n");
        failures++;
    }

    if (selftest_execute_command("power-source apply\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "error permission denied")) {
        fprintf(stderr, "self-test failed: denied power-source apply command path\n");
        failures++;
    }

    if (selftest_execute_command("power-source auto on\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "error permission denied")) {
        fprintf(stderr, "self-test failed: denied power-source auto command path\n");
        failures++;
    }

    if (selftest_execute_command("power-source apply\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "power_source=battery profile=quiet") ||
        ec_read_byte(ec, cfg->platform_profiles.reg) != 0x00) {
        fprintf(stderr, "self-test failed: power-source battery apply command path\n");
        failures++;
    }

    if (selftest_execute_command("power-source auto on\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "power_source=battery auto_apply=on profile=quiet") ||
        ec_read_byte(ec, cfg->platform_profiles.reg) != 0x00) {
        fprintf(stderr, "self-test failed: power-source auto on command path\n");
        failures++;
    }

    if (selftest_execute_command("power-source auto off\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "power_source=battery auto_apply=off profile=quiet")) {
        fprintf(stderr, "self-test failed: power-source auto off command path\n");
        failures++;
    }

    setenv("ANS_FAKE_POWER_SOURCE", "ac", 1);
    if (selftest_execute_command("power-source apply\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "power_source=ac profile=balanced") ||
        ec_read_byte(ec, cfg->platform_profiles.reg) != 0x01) {
        fprintf(stderr, "self-test failed: power-source AC apply command path\n");
        failures++;
    }
    unsetenv("ANS_FAKE_POWER_SOURCE");

    return failures;
}
