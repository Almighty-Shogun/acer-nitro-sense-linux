#include "cases.h"

#include "../helpers.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>

int unit_run_daemon_lifecycle_commands(struct ec_device *ec,
                                       const struct ans_config *cfg,
                                       fan_state states[ANS_MAX_FANS],
                                       bool *auto_mode, char *preset,
                                       const size_t preset_len,
                                       bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    if (unit_execute_command("resume\n", ec, cfg, states, auto_mode,
                             preset, preset_len, coolboost_enabled,
                             true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "resume=ok mode=auto preset=auto")) {
        fprintf(stderr, "unit-test failed: resume command path\n");
        failures++;
    }

    daemon_running = 1;
    if (unit_execute_command("stop\n", ec, cfg, states, auto_mode,
                             preset, preset_len, coolboost_enabled,
                             true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "stop=ok reset=firmware") || daemon_running != 0) {
        fprintf(stderr, "unit-test failed: stop command path\n");
        failures++;
    }
    daemon_running = 1;

    return failures;
}
