#include "selftest/platform_cases.h"

#include "platform/control.h"
#include "selftest/helpers.h"

#include <stdio.h>
#include <string.h>

int selftest_run_platform_fan_mode_commands(struct ec_device *ec,
                                            const struct ans_config *cfg,
                                            fan_state states[ANS_MAX_FANS],
                                            bool *auto_mode, char *preset,
                                            const size_t preset_len,
                                            bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    if (selftest_execute_command("coolboost on\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "error permission denied")) {
        fprintf(stderr, "self-test failed: denied coolboost command path\n");
        failures++;
    }

    if (selftest_execute_command("coolboost on\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "coolboost=on") ||
        !*coolboost_enabled ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_turbo_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_turbo_value) {
        fprintf(stderr, "self-test failed: coolboost on command path\n");
        failures++;
    }

    if (selftest_execute_command("coolboost off\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "coolboost=off") ||
        *coolboost_enabled ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_manual_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_manual_value) {
        fprintf(stderr, "self-test failed: coolboost off command path\n");
        failures++;
    }

    if (selftest_execute_command("fan-mode turbo\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "fan_mode=turbo") ||
        !*coolboost_enabled ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_turbo_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_turbo_value) {
        fprintf(stderr, "self-test failed: fan-mode turbo command path\n");
        failures++;
    }

    if (selftest_execute_command("fan-mode status\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, false, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "fan_mode=turbo")) {
        fprintf(stderr, "self-test failed: fan-mode status command path\n");
        failures++;
    }

    if (selftest_execute_command("fan-mode manual\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "fan_mode=manual") ||
        *coolboost_enabled ||
        *auto_mode || strcmp(preset, "manual") != 0 ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_manual_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_manual_value) {
        fprintf(stderr, "self-test failed: fan-mode manual command path\n");
        failures++;
    }

    if (selftest_execute_command("fan-mode auto\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "fan_mode=auto") ||
        *auto_mode || strcmp(preset, FIRMWARE_AUTO_PRESET) != 0 ||
        *coolboost_enabled ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_auto_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_auto_value) {
        fprintf(stderr, "self-test failed: fan-mode auto command path\n");
        failures++;
    }

    return failures;
}
