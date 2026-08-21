#include "unit/daemon.h"

#include "daemon/types.h"
#include "ec/ec.h"
#include "unit/commands.h"
#include "unit/daemon_args.h"
#include "unit/fan_control.h"
#include "unit/parser.h"
#include "unit/state.h"
#include "unit/fixture.h"
#include "unit/utility.h"

#include <stdio.h>

int run_unit_tests(void)
{
    daemon_quiet_logs = true;
    daemon_persist_control_state = false;
    int failures = unit_run_parser();

    failures += unit_run_daemon_args();
    failures += unit_run_utility_helpers();

    struct ans_config cfg;
    struct ec_device ec = {.backend = EC_BACKEND_NONE};
    fan_state states[ANS_MAX_FANS];

    init_unit_test_config(&cfg);
    reset_unit_test_states(&cfg, states);
    if (ec_open_rw("fake", &ec) < 0) {
        fprintf(stderr, "unit-test failed: open fake EC\n");
        failures++;
    }

    if (failures == 0) {
        bool coolboost_enabled = cfg.coolboost.default_enabled;

        failures += unit_run_fan_control(&ec, &cfg, states);
        failures += unit_run_state_restore(&ec, &cfg, states,
                                               &coolboost_enabled);
        failures += unit_run_daemon_commands(&ec, &cfg, states);
    }

    if (ec.backend != EC_BACKEND_NONE)
        ec_close(&ec);

    if (failures > 0)
        return 1;

    printf("unit-test ok\n");
    return 0;
}
