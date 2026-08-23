#include "daemon.h"

#include "args.h"
#include "ec/ec.h"
#include "state.h"
#include "../fixture.h"
#include "../utility.h"
#include "status_json.h"
#include "daemon/types.h"
#include "../fan/control.h"
#include "../parser/parser.h"
#include "../commands/commands.h"

#include <stdio.h>

/**
 * Run daemon command tests.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
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

    if (ec_open_rw("fake", &ec) < 0)
    {
        fprintf(stderr, "unit-test failed: open fake EC\n");

        failures++;
    }

    if (failures == 0)
    {
        bool coolboost_enabled = cfg.coolboost.default_enabled;

        failures += unit_run_fan_control(&ec, &cfg, states);
        failures += unit_run_status_json(&ec, &cfg, states);
        failures += unit_run_state_restore(&ec, &cfg, states, &coolboost_enabled);
        failures += unit_run_daemon_commands(&ec, &cfg, states);
    }

    if (ec.backend != EC_BACKEND_NONE)
        ec_close(&ec);

    if (failures > 0)
        return 1;

    printf("unit-test ok\n");

    return 0;
}
