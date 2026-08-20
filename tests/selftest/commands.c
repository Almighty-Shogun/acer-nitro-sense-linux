#include "selftest/commands.h"

#include "selftest/command_cases.h"
#include "selftest/platform_commands.h"
#include "selftest/fixture.h"

int selftest_run_daemon_commands(struct ec_device *ec, const struct ans_config *cfg,
                                 fan_state states[ANS_MAX_FANS])
{
    int failures = 0;
    bool auto_mode = false;
    bool coolboost_enabled = false;
    char preset[32] = "manual";

    failures += selftest_run_platform_commands(ec, cfg, states);
    reset_self_test_states(cfg, states);

    failures += selftest_run_ec_debug_commands(ec, cfg, states, &auto_mode,
                                               preset, sizeof(preset),
                                               &coolboost_enabled);
    failures += selftest_run_fan_socket_commands(ec, cfg, states, &auto_mode,
                                                 preset, sizeof(preset),
                                                 &coolboost_enabled);
    failures += selftest_run_firmware_auto_commands(ec, cfg, states,
                                                    &auto_mode, preset,
                                                    sizeof(preset),
                                                    &coolboost_enabled);
    failures += selftest_run_invalid_and_reset_commands(ec, cfg, states,
                                                        &auto_mode, preset,
                                                        sizeof(preset),
                                                        &coolboost_enabled);

    return failures;
}
