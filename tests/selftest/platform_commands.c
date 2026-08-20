#include "selftest/platform_commands.h"

#include "selftest/platform_cases.h"
#include "platform/control.h"
#include "selftest/fixture.h"

int selftest_run_platform_commands(struct ec_device *ec,
                                   const struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS])
{
    int failures = 0;
    bool auto_mode = false;
    bool coolboost_enabled = false;
    char preset[32] = "manual";

    reset_self_test_states(cfg, states);
    apply_coolboost(ec, cfg, states, coolboost_enabled);

    failures += selftest_run_platform_status_commands(ec, cfg, states,
                                                      &auto_mode, preset,
                                                      sizeof(preset),
                                                      &coolboost_enabled);
    failures += selftest_run_platform_fan_mode_commands(ec, cfg, states,
                                                        &auto_mode, preset,
                                                        sizeof(preset),
                                                        &coolboost_enabled);
    failures += selftest_run_platform_power_source_commands(ec, cfg, states,
                                                            &auto_mode, preset,
                                                            sizeof(preset),
                                                            &coolboost_enabled);
    failures += selftest_run_platform_keyboard_backlight_commands(
        ec, cfg, states, &auto_mode, preset, sizeof(preset), &coolboost_enabled);

    return failures;
}
