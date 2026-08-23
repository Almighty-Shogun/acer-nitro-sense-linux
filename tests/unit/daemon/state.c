#include "state.h"

#include "ec/ec.h"
#include "../fixture.h"
#include "daemon/state.h"
#include "platform/control.h"

#include <stdio.h>
#include <string.h>

typedef struct
{
    struct ec_device* ec;
    const struct ans_config* cfg;
    fan_state* states;
    bool* auto_mode;
    char* preset;
    size_t preset_len;
    bool* coolboost_enabled;
    daemon_runtime_state* runtime;
} state_restore_context;

/**
 * Restore one control-state JSON payload into the shared test context.
 *
 * The restore tests all use the same runtime objects, so this wrapper keeps the
 * assertion body focused on the expected state after parsing.
 */
static bool restore_state_json(const state_restore_context* ctx, const char* json)
{
    return restore_control_state_from_json(
        ctx->ec,
        ctx->cfg,
        ctx->states,
        ctx->auto_mode,
        ctx->preset,
        ctx->preset_len,
        ctx->coolboost_enabled,
        ctx->runtime,
        json
    );
}

/**
 * Return whether the restored fan percentages match the expected values.
 *
 * State persistence stores requested fan percentages separately from the EC
 * write registers, so tests verify both surfaces explicitly.
 */
static bool fan_state_percent_matches(fan_state states[ANS_MAX_FANS], const int cpu_percent, const int gpu_percent)
{
    const bool cpu_percent_ok = states[0].percent == cpu_percent;
    const bool gpu_percent_ok = states[1].percent == gpu_percent;

    return cpu_percent_ok && gpu_percent_ok;
}

/**
 * Return whether the EC fan write registers match expected values.
 *
 * Manual and auto restores write fan speed registers, while firmware-auto must
 * leave existing speed registers untouched.
 */
static bool fan_write_registers_match(struct ec_device* ec, const struct ans_config* cfg, const int cpu_value, const int gpu_value)
{
    const bool cpu_write_ok = ec_read_byte(ec, cfg->fans[0].write_register) == cpu_value;
    const bool gpu_write_ok = ec_read_byte(ec, cfg->fans[1].write_register) == gpu_value;

    return cpu_write_ok && gpu_write_ok;
}

/**
 * Return whether fan-mode registers are set to manual mode.
 *
 * Manual control and normal auto restores both require the daemon to retain
 * manual fan-mode ownership.
 */
static bool fan_mode_registers_manual(struct ec_device* ec, const struct ans_config* cfg)
{
    const bool cpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.cpu_reg) == cfg->fan_modes.cpu_manual_value;
    const bool gpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.gpu_reg) == cfg->fan_modes.gpu_manual_value;

    return cpu_mode_ok && gpu_mode_ok;
}

/**
 * Return whether fan-mode registers are set to firmware-auto mode.
 *
 * Firmware-auto restores should hand fan mode control back to the embedded
 * controller.
 */
static bool fan_mode_registers_auto(struct ec_device* ec, const struct ans_config* cfg)
{
    const bool cpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.cpu_reg) == cfg->fan_modes.cpu_auto_value;
    const bool gpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.gpu_reg) == cfg->fan_modes.gpu_auto_value;

    return cpu_mode_ok && gpu_mode_ok;
}

/**
 * Verify daemon state restoration.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_state_restore(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* coolboost_enabled
)
{
    int failures = 0;
    bool auto_mode = false;
    char preset[32] = "manual";

    daemon_runtime_state runtime = {
        .power_source_auto_apply = cfg->power_source_profiles.auto_apply,
    };

    const state_restore_context ctx = {
        .ec = ec,
        .cfg = cfg,
        .states = states,
        .auto_mode = &auto_mode,
        .preset = preset,
        .preset_len = sizeof(preset),
        .coolboost_enabled = coolboost_enabled,
        .runtime = &runtime,
    };

    reset_unit_test_states(cfg, states);
    runtime.power_source_auto_apply = false;

    const char* auto_json =
        "{ \"auto\": true, \"preset\": \"manual\", \"coolboost\": true,"
        "\"power_source_auto_apply\": true, \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 72 },"
        "{ \"id\": \"gpu\", \"percent\": 64 }"
        "] }";

    const bool auto_restore_parsed = restore_state_json(&ctx, auto_json);
    const bool auto_mode_ok = auto_mode && strcmp(preset, "auto") == 0;

    const bool auto_flags_ok = !*coolboost_enabled && runtime.power_source_auto_apply;

    const bool auto_fans_ok = fan_state_percent_matches(states, 72, 64);
    const bool auto_writes_ok = fan_write_registers_match(ec, cfg, 72, 64);
    const bool auto_fan_mode_ok = fan_mode_registers_manual(ec, cfg);

    const bool auto_restore_ok = auto_restore_parsed
                                 && auto_mode_ok
                                 && auto_flags_ok
                                 && auto_fans_ok
                                 && auto_writes_ok
                                 && auto_fan_mode_ok;

    if (!auto_restore_ok)
    {
        fprintf(stderr, "unit-test failed: auto state restore\n");

        failures++;
    }

    reset_unit_test_states(cfg, states);

    runtime.power_source_auto_apply = true;
    auto_mode = true;

    *coolboost_enabled = true;

    snprintf(preset, sizeof(preset), "auto");

    const char* manual_json =
        "{ \"auto\": false, \"preset\": \"manual\", \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 66 },"
        "{ \"id\": \"gpu\", \"percent\": 61 }"
        "] }";

    const bool manual_restore_parsed = restore_state_json(&ctx, manual_json);
    const bool manual_mode_ok = !auto_mode && strcmp(preset, "manual") == 0;

    const bool manual_flags_ok = !*coolboost_enabled;

    const bool manual_fans_ok = fan_state_percent_matches(states, 66, 61);
    const bool manual_writes_ok = fan_write_registers_match(ec, cfg, 66, 61);
    const bool manual_fan_mode_ok = fan_mode_registers_manual(ec, cfg);

    const bool manual_restore_ok = manual_restore_parsed
                                   && manual_mode_ok
                                   && manual_flags_ok
                                   && manual_fans_ok
                                   && manual_writes_ok
                                   && manual_fan_mode_ok;

    if (!manual_restore_ok)
    {
        fprintf(stderr, "unit-test failed: manual state restore\n");

        failures++;
    }

    reset_unit_test_states(cfg, states);

    runtime.power_source_auto_apply = false;
    auto_mode = true;

    snprintf(preset, sizeof(preset), "auto");

    const char* preset_json =
        "{ \"auto\": false, \"preset\": \"balanced\", \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 99 },"
        "{ \"id\": \"gpu\", \"percent\": 99 }"
        "] }";

    const bool preset_restore_parsed = restore_state_json(&ctx, preset_json);
    const bool preset_mode_ok = !auto_mode && strcmp(preset, "balanced") == 0;

    const bool preset_fans_ok = fan_state_percent_matches(states, 50, 45);
    const bool preset_writes_ok = fan_write_registers_match(ec, cfg, 50, 45);

    const bool preset_restore_ok = preset_restore_parsed && preset_mode_ok && preset_fans_ok && preset_writes_ok;

    if (!preset_restore_ok)
    {
        fprintf(stderr, "unit-test failed: preset state restore\n");

        failures++;
    }

    reset_unit_test_states(cfg, states);

    runtime.power_source_auto_apply = false;
    auto_mode = true;

    *coolboost_enabled = true;

    snprintf(preset, sizeof(preset), "auto");

    ec_write_byte(ec, cfg->fans[0].write_register, 33);
    ec_write_byte(ec, cfg->fans[1].write_register, 44);

    const char* firmware_auto_json =
        "{ \"auto\": false, \"preset\": \"firmware-auto\", \"fans\": ["
        "{ \"id\": \"cpu\", \"percent\": 72 },"
        "{ \"id\": \"gpu\", \"percent\": 64 }"
        "] }";

    const bool firmware_restore_parsed = restore_state_json(&ctx, firmware_auto_json);
    const bool firmware_mode_ok = !auto_mode && strcmp(preset, FIRMWARE_AUTO_PRESET) == 0;

    const bool firmware_flags_ok = !*coolboost_enabled;

    const bool firmware_fans_ok = fan_state_percent_matches(states, 72, 64);
    const bool firmware_writes_ok = fan_write_registers_match(ec, cfg, 33, 44);
    const bool firmware_fan_mode_ok = fan_mode_registers_auto(ec, cfg);

    const bool firmware_restore_ok = firmware_restore_parsed
                                     && firmware_mode_ok
                                     && firmware_flags_ok
                                     && firmware_fans_ok
                                     && firmware_writes_ok
                                     && firmware_fan_mode_ok;

    if (!firmware_restore_ok)
    {
        fprintf(stderr, "unit-test failed: firmware-auto state restore\n");

        failures++;
    }

    return failures;
}
