#ifndef ANS_DAEMON_UNIT_COMMANDS_H
#define ANS_DAEMON_UNIT_COMMANDS_H

#include "daemon/types.h"

/**
 * Run all daemon command unit tests.
 *
 * The grouped runner shares one fake EC device, config, and fan-state array
 * across command-case modules.
 */
int unit_run_daemon_commands(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

#endif
