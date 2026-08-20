#include "selftest/daemon.h"

#include "ans.h"
#include "selftest/commands.h"
#include "selftest/fan_control.h"
#include "selftest/parser.h"
#include "selftest/state.h"
#include "daemon/types.h"
#include "selftest/fixture.h"

#include <stdio.h>

int run_self_tests(void)
{
    daemon_quiet_logs = true;
    daemon_persist_control_state = false;
    int failures = selftest_run_parser();
    struct ans_config cfg;
    struct ec_device ec = {.backend = EC_BACKEND_NONE};
    fan_state states[ANS_MAX_FANS];

    init_self_test_config(&cfg);
    reset_self_test_states(&cfg, states);
    if (ec_open_rw("fake", &ec) < 0) {
        fprintf(stderr, "self-test failed: open fake EC\n");
        failures++;
    }

    if (failures == 0) {
        bool coolboost_enabled = cfg.coolboost.default_enabled;

        failures += selftest_run_fan_control(&ec, &cfg, states);
        failures += selftest_run_state_restore(&ec, &cfg, states,
                                               &coolboost_enabled);
        failures += selftest_run_daemon_commands(&ec, &cfg, states);
    }

    if (ec.backend != EC_BACKEND_NONE)
        ec_close(&ec);

    if (failures > 0)
        return 1;

    printf("self-test ok\n");
    return 0;
}
