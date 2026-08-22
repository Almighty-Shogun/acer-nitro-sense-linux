#include "unit/commands.h"

#include "unit/command_cases.h"
#include "unit/platform_commands.h"
#include "unit/fixture.h"

int unit_run_daemon_commands(struct ec_device *ec, const struct ans_config *cfg,
                                 fan_state states[ANS_MAX_FANS])
{
    int failures = 0;
    bool auto_mode = false;
    bool coolboost_enabled = false;
    char preset[32] = "manual";

    failures += unit_run_platform_commands(ec, cfg, states);
    reset_unit_test_states(cfg, states);

    failures += unit_run_ec_debug_commands(ec, cfg, states, &auto_mode,
                                               preset, sizeof(preset),
                                               &coolboost_enabled);
    failures += unit_run_fan_socket_commands(ec, cfg, states, &auto_mode,
                                                 preset, sizeof(preset),
                                                 &coolboost_enabled);
    failures += unit_run_daemon_lifecycle_commands(ec, cfg, states, &auto_mode,
                                                       preset, sizeof(preset),
                                                       &coolboost_enabled);
    failures += unit_run_firmware_auto_commands(ec, cfg, states,
                                                    &auto_mode, preset,
                                                    sizeof(preset),
                                                    &coolboost_enabled);
    failures += unit_run_invalid_and_reset_commands(ec, cfg, states,
                                                        &auto_mode, preset,
                                                        sizeof(preset),
                                                        &coolboost_enabled);

    return failures;
}
