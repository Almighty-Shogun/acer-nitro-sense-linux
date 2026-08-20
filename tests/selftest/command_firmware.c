#include "selftest/command_cases.h"

#include "fan/control.h"
#include "platform/control.h"
#include "selftest/fixture.h"
#include "selftest/helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int selftest_run_firmware_auto_commands(struct ec_device *ec,
                                        const struct ans_config *cfg,
                                        fan_state states[ANS_MAX_FANS],
                                        bool *auto_mode, char *preset,
                                        const size_t preset_len,
                                        bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    ec_write_byte(ec, cfg->fans[0].write_register, 31);
    ec_write_byte(ec, cfg->fans[1].write_register, 41);
    if (selftest_execute_command("firmware-auto\n", ec, cfg, states,
                                 auto_mode, preset, preset_len,
                                 coolboost_enabled, true, reply,
                                 sizeof(reply)) < 0 ||
        !strstr(reply, "mode=firmware-auto preset=firmware-auto") ||
        *auto_mode || strcmp(preset, FIRMWARE_AUTO_PRESET) != 0 ||
        *coolboost_enabled ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_auto_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_auto_value) {
        fprintf(stderr, "self-test failed: firmware-auto command path\n");
        failures++;
    }
    update_fan_states(ec, cfg, states, *auto_mode, preset);
    if (ec_read_byte(ec, cfg->fans[0].write_register) != 31 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 41) {
        fprintf(stderr, "self-test failed: firmware-auto update wrote speed registers\n");
        failures++;
    }
    if (selftest_execute_command("status\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "control=firmware active_percent=firmware") ||
        !strstr(reply, "safety=ok")) {
        fprintf(stderr, "status reply:\n%s", reply);
        fprintf(stderr, "self-test failed: firmware-auto status clarity\n");
        failures++;
    }

    reset_self_test_states(cfg, states);
    *auto_mode = false;
    snprintf(preset, preset_len, "%s", FIRMWARE_AUTO_PRESET);
    ec_write_byte(ec, cfg->fan_modes.cpu_reg, cfg->fan_modes.cpu_auto_value);
    ec_write_byte(ec, cfg->fan_modes.gpu_reg, cfg->fan_modes.gpu_auto_value);
    ec_write_byte(ec, cfg->fans[0].write_register, 31);
    ec_write_byte(ec, cfg->fans[1].write_register, 41);
    setenv("ANS_FAKE_CPU_TEMP_C", "92", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "60", 1);
    update_fan_states(ec, cfg, states, *auto_mode, preset);
    update_fan_states(ec, cfg, states, *auto_mode, preset);
    if (ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_manual_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_manual_value ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 100 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 100 ||
        strcmp(states[0].safety_reason, "critical-temperature") != 0 ||
        strcmp(states[1].safety_reason, "critical-temperature") != 0) {
        fprintf(stderr, "self-test failed: firmware-auto critical safety takeover\n");
        failures++;
    }

    setenv("ANS_FAKE_CPU_TEMP_C", "55", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "50", 1);
    update_fan_states(ec, cfg, states, *auto_mode, preset);
    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");
    if (ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_auto_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_auto_value ||
        states[0].percent != states[0].requested_percent ||
        states[1].percent != states[1].requested_percent ||
        strcmp(states[0].safety_reason, "") != 0 ||
        strcmp(states[1].safety_reason, "") != 0) {
        fprintf(stderr, "self-test failed: firmware-auto safety restore\n");
        failures++;
    }

    return failures;
}
