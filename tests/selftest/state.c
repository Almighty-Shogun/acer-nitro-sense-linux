#include "selftest/state.h"

#include "daemon/state.h"
#include "platform/control.h"
#include "selftest/fixture.h"

#include <stdio.h>
#include <string.h>

int selftest_run_state_restore(struct ec_device *ec, const struct ans_config *cfg,
                               fan_state states[ANS_MAX_FANS],
                               bool *coolboost_enabled)
{
    int failures = 0;
    bool auto_mode = false;
    char preset[32] = "manual";
    daemon_runtime_state runtime = {
        .power_source_auto_apply = cfg->power_source_profiles.auto_apply,
    };

    reset_self_test_states(cfg, states);
    runtime.power_source_auto_apply = false;
    const char *auto_json =
        "{ \"auto\": true, \"preset\": \"manual\", \"coolboost\": true,"
        "\"power_source_auto_apply\": true, \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 72 },"
        "{ \"id\": \"gpu\", \"percent\": 64 }"
        "] }";
    if (!restore_control_state_from_json(ec, cfg, states, &auto_mode, preset,
                                         sizeof(preset), coolboost_enabled,
                                         &runtime,
                                         auto_json) ||
        !auto_mode || strcmp(preset, "auto") != 0 ||
        *coolboost_enabled ||
        !runtime.power_source_auto_apply ||
        states[0].percent != 72 || states[1].percent != 64 ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 72 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 64 ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_manual_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_manual_value) {
        fprintf(stderr, "self-test failed: auto state restore\n");
        failures++;
    }

    reset_self_test_states(cfg, states);
    runtime.power_source_auto_apply = true;
    auto_mode = true;
    *coolboost_enabled = true;
    snprintf(preset, sizeof(preset), "auto");
    const char *manual_json =
        "{ \"auto\": false, \"preset\": \"manual\", \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 66 },"
        "{ \"id\": \"gpu\", \"percent\": 61 }"
        "] }";
    if (!restore_control_state_from_json(ec, cfg, states, &auto_mode, preset,
                                         sizeof(preset), coolboost_enabled,
                                         &runtime,
                                         manual_json) ||
        auto_mode || strcmp(preset, "manual") != 0 ||
        *coolboost_enabled ||
        states[0].percent != 66 || states[1].percent != 61 ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 66 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 61 ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_manual_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_manual_value) {
        fprintf(stderr, "self-test failed: manual state restore\n");
        failures++;
    }

    reset_self_test_states(cfg, states);
    runtime.power_source_auto_apply = false;
    auto_mode = true;
    snprintf(preset, sizeof(preset), "auto");
    const char *preset_json =
        "{ \"auto\": false, \"preset\": \"balanced\", \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 99 },"
        "{ \"id\": \"gpu\", \"percent\": 99 }"
        "] }";
    if (!restore_control_state_from_json(ec, cfg, states, &auto_mode, preset,
                                         sizeof(preset), coolboost_enabled,
                                         &runtime,
                                         preset_json) ||
        auto_mode || strcmp(preset, "balanced") != 0 ||
        states[0].percent != 50 || states[1].percent != 45 ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 50 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 45) {
        fprintf(stderr, "self-test failed: preset state restore\n");
        failures++;
    }

    reset_self_test_states(cfg, states);
    runtime.power_source_auto_apply = false;
    auto_mode = true;
    *coolboost_enabled = true;
    snprintf(preset, sizeof(preset), "auto");
    ec_write_byte(ec, cfg->fans[0].write_register, 33);
    ec_write_byte(ec, cfg->fans[1].write_register, 44);
    const char *firmware_auto_json =
        "{ \"auto\": false, \"preset\": \"firmware-auto\", \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 72 },"
        "{ \"id\": \"gpu\", \"percent\": 64 }"
        "] }";
    if (!restore_control_state_from_json(ec, cfg, states, &auto_mode, preset,
                                         sizeof(preset), coolboost_enabled,
                                         &runtime,
                                         firmware_auto_json) ||
        auto_mode || strcmp(preset, FIRMWARE_AUTO_PRESET) != 0 ||
        *coolboost_enabled ||
        states[0].percent != 72 || states[1].percent != 64 ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 33 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 44 ||
        ec_read_byte(ec, cfg->fan_modes.cpu_reg) != cfg->fan_modes.cpu_auto_value ||
        ec_read_byte(ec, cfg->fan_modes.gpu_reg) != cfg->fan_modes.gpu_auto_value) {
        fprintf(stderr, "self-test failed: firmware-auto state restore\n");
        failures++;
    }

    return failures;
}
