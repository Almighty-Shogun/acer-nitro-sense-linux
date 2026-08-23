#include "cases.h"

#include "ec/ec.h"
#include "../fixture.h"
#include "../helpers.h"
#include "fan/control.h"
#include "platform/control.h"

#include <stdio.h>
#include <stdlib.h>
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
    char* reply;
    size_t reply_len;
} firmware_auto_context;

/**
 * Execute a firmware-auto test command and match its reply.
 *
 * Firmware-auto tests reuse the same runtime state across command and status
 * assertions, so this helper keeps each check focused on the command contract.
 */
static bool firmware_auto_command_matches(firmware_auto_context* ctx, const char* command, const char* expected)
{
    const int result = unit_execute_command(
        command,
        ctx->ec,
        ctx->cfg,
        ctx->states,
        ctx->auto_mode,
        ctx->preset,
        ctx->preset_len,
        ctx->coolboost_enabled,
        true,
        ctx->reply,
        ctx->reply_len
    );

    return result >= 0 && strstr(ctx->reply, expected);
}

/**
 * Return whether firmware-auto mode is applied to both fan-mode registers.
 *
 * Firmware-auto should return control to the embedded controller instead of
 * keeping manual values in the fan-mode registers.
 */
static bool firmware_auto_registers_active(struct ec_device* ec, const struct ans_config* cfg)
{
    const bool cpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.cpu_reg) == cfg->fan_modes.cpu_auto_value;
    const bool gpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.gpu_reg) == cfg->fan_modes.gpu_auto_value;

    return cpu_mode_ok && gpu_mode_ok;
}

/**
 * Return whether manual fan-mode registers are active.
 *
 * Critical safety temporarily takes firmware-auto out of firmware control so
 * the daemon can force safe manual fan speeds.
 */
static bool firmware_manual_registers_active(struct ec_device* ec, const struct ans_config* cfg)
{
    const bool cpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.cpu_reg) == cfg->fan_modes.cpu_manual_value;
    const bool gpu_mode_ok = ec_read_byte(ec, cfg->fan_modes.gpu_reg) == cfg->fan_modes.gpu_manual_value;

    return cpu_mode_ok && gpu_mode_ok;
}

/**
 * Return whether both fans report the same safety reason.
 *
 * Firmware-auto safety takeover is shared across the CPU and GPU fan states, so
 * the test verifies both reasons together.
 */
static bool fan_safety_reasons_match(fan_state states[ANS_MAX_FANS], const char* reason)
{
    const bool cpu_reason_ok = strcmp(states[0].safety_reason, reason) == 0;
    const bool gpu_reason_ok = strcmp(states[1].safety_reason, reason) == 0;

    return cpu_reason_ok && gpu_reason_ok;
}

/**
 * Verify firmware-auto command behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_firmware_auto_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled
)
{
    int failures = 0;
    char reply[1024];

    firmware_auto_context ctx = {
        .ec = ec,
        .cfg = cfg,
        .states = states,
        .auto_mode = auto_mode,
        .preset = preset,
        .preset_len = preset_len,
        .coolboost_enabled = coolboost_enabled,
        .reply = reply,
        .reply_len = sizeof(reply),
    };

    ec_write_byte(ec, cfg->fans[0].write_register, 31);
    ec_write_byte(ec, cfg->fans[1].write_register, 41);

    setenv("ANS_FAKE_CPU_TEMP_C", "45", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "45", 1);

    const bool firmware_reply_ok = firmware_auto_command_matches(&ctx, "firmware-auto\n", "mode=firmware-auto preset=firmware-auto");

    const bool firmware_mode_ok = !*auto_mode;
    const bool firmware_preset_ok = strcmp(preset, FIRMWARE_AUTO_PRESET) == 0;

    const bool firmware_state_ok = firmware_mode_ok && firmware_preset_ok && !*coolboost_enabled;

    const bool firmware_registers_ok = firmware_auto_registers_active(ec, cfg);
    const bool firmware_auto_ok = firmware_reply_ok && firmware_state_ok && firmware_registers_ok;

    if (!firmware_auto_ok)
    {
        fprintf(stderr, "unit-test failed: firmware-auto command path\n");

        failures++;
    }

    update_fan_states(ec, cfg, states, *auto_mode, preset);

    const bool cpu_speed_unchanged = ec_read_byte(ec, cfg->fans[0].write_register) == 31;
    const bool gpu_speed_unchanged = ec_read_byte(ec, cfg->fans[1].write_register) == 41;

    const bool speed_registers_unchanged = cpu_speed_unchanged && gpu_speed_unchanged;

    if (!speed_registers_unchanged)
    {
        fprintf(stderr, "unit-test failed: firmware-auto update wrote speed registers\n");

        failures++;
    }

    const bool status_firmware_ok = firmware_auto_command_matches(&ctx, "status\n", "control=firmware active_percent=firmware");

    const bool status_safety_ok = strstr(reply, "safety=ok");
    const bool status_ok = status_firmware_ok && status_safety_ok;

    if (!status_ok)
    {
        fprintf(stderr, "status reply:\n%s", reply);
        fprintf(stderr, "unit-test failed: firmware-auto status clarity\n");

        failures++;
    }

    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");

    reset_unit_test_states(cfg, states);

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

    const bool safety_manual_ok = firmware_manual_registers_active(ec, cfg);

    const bool cpu_safety_speed_ok = ec_read_byte(ec, cfg->fans[0].write_register) == 100;
    const bool gpu_safety_speed_ok = ec_read_byte(ec, cfg->fans[1].write_register) == 100;

    const bool safety_speed_ok = cpu_safety_speed_ok && gpu_safety_speed_ok;
    const bool safety_reason_ok = fan_safety_reasons_match(states, "critical-temperature");

    const bool safety_takeover_ok = safety_manual_ok && safety_speed_ok && safety_reason_ok;

    if (!safety_takeover_ok)
    {
        fprintf(stderr, "unit-test failed: firmware-auto critical safety takeover\n");

        failures++;
    }

    setenv("ANS_FAKE_CPU_TEMP_C", "55", 1);
    setenv("ANS_FAKE_GPU_TEMP_C", "50", 1);

    update_fan_states(ec, cfg, states, *auto_mode, preset);

    unsetenv("ANS_FAKE_CPU_TEMP_C");
    unsetenv("ANS_FAKE_GPU_TEMP_C");

    const bool restore_registers_ok = firmware_auto_registers_active(ec, cfg);

    const bool cpu_percent_restored = states[0].percent == states[0].requested_percent;
    const bool gpu_percent_restored = states[1].percent == states[1].requested_percent;

    const bool restore_percent_ok = cpu_percent_restored && gpu_percent_restored;
    const bool restore_reason_ok = fan_safety_reasons_match(states, "");
    const bool safety_restore_ok = restore_registers_ok && restore_percent_ok && restore_reason_ok;

    if (!safety_restore_ok)
    {
        fprintf(stderr, "unit-test failed: firmware-auto safety restore\n");

        failures++;
    }

    return failures;
}
