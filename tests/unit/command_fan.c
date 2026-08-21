#include "unit/command_cases.h"

#include "ec/ec.h"
#include "unit/helpers.h"

#include <stdio.h>
#include <string.h>

int unit_run_fan_socket_commands(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     fan_state states[ANS_MAX_FANS],
                                     bool *auto_mode, char *preset,
                                     const size_t preset_len,
                                     bool *coolboost_enabled)
{
    int failures = 0;
    char reply[1024];

    if (unit_execute_command("set cpu 44\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 false, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "error permission denied")) {
        fprintf(stderr, "unit-test failed: denied set command path\n");
        failures++;
    }

    if (unit_execute_command("set cpu 44\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "mode=manual fan=cpu requested=44") ||
        *auto_mode || strcmp(preset, "manual") != 0 ||
        states[0].percent != 44 ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 44) {
        fprintf(stderr, "unit-test failed: set cpu command path\n");
        failures++;
    }

    if (unit_execute_command("set all 48\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "mode=manual fan=all requested=48") ||
        states[0].percent != 48 || states[1].percent != 48 ||
        ec_read_byte(ec, cfg->fans[0].write_register) != 48 ||
        ec_read_byte(ec, cfg->fans[1].write_register) != 48) {
        fprintf(stderr, "unit-test failed: set all command path\n");
        failures++;
    }

    if (unit_execute_command("preset max\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "mode=preset preset=max cpu=100 gpu=100") ||
        *auto_mode || strcmp(preset, "max") != 0 ||
        states[0].percent != 100 || states[1].percent != 100) {
        fprintf(stderr, "unit-test failed: preset command path\n");
        failures++;
    }

    if (unit_execute_command("auto\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "mode=auto preset=auto") ||
        !*auto_mode || strcmp(preset, "auto") != 0 ||
        states[0].percent != 50 || states[1].percent != 50) {
        fprintf(stderr, "unit-test failed: auto command path\n");
        failures++;
    }

    if (unit_execute_command("resume\n", ec, cfg, states, auto_mode,
                                 preset, preset_len, coolboost_enabled,
                                 true, reply, sizeof(reply)) < 0 ||
        !strstr(reply, "resume=ok mode=auto preset=auto")) {
        fprintf(stderr, "unit-test failed: resume command path\n");
        failures++;
    }

    return failures;
}
