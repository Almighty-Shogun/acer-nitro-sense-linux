#include "commands.h"

#include "cases.h"
#include "../fixture.h"
#include "platform/control.h"

/**
 * Run platform command tests.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_platform_commands(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    int failures = 0;

    bool auto_mode = false;
    bool coolboost_enabled = false;

    char preset[32] = "manual";

    reset_unit_test_states(cfg, states);
    apply_coolboost(ec, cfg, states, coolboost_enabled);

    failures += unit_run_platform_status_commands(ec, cfg, states, &auto_mode, preset, sizeof(preset), &coolboost_enabled);
    failures += unit_run_platform_fan_mode_commands(ec, cfg, states, &auto_mode, preset, sizeof(preset), &coolboost_enabled);
    failures += unit_run_platform_power_source_commands(ec, cfg, states, &auto_mode, preset, sizeof(preset), &coolboost_enabled);

    failures += unit_run_platform_keyboard_backlight_commands(
        ec,
        cfg,
        states,
        &auto_mode,
        preset,
        sizeof(preset),
        &coolboost_enabled
    );

    return failures;
}
