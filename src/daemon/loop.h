#ifndef ANS_DAEMON_LOOP_H
#define ANS_DAEMON_LOOP_H

#include "daemon/types.h"
#include "hardware/hardware.h"

/**
 * Run the daemon polling and control loop.
 *
 * The loop refreshes sensors, reconciles fan state, services control-socket
 * clients, and applies optional runtime policies.
 */
void run_daemon_loop(
    int sock_fd,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled,
    const hardware_names* names,
    daemon_runtime_state* runtime
);

#endif
