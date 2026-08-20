#ifndef ANS_DAEMON_LOOP_H
#define ANS_DAEMON_LOOP_H

#include "daemon/types.h"
#include "hardware/hardware.h"

void run_daemon_loop(int sock_fd, struct ec_device *ec, const struct ans_config *cfg,
                     fan_state states[ANS_MAX_FANS], bool *auto_mode,
                     char *preset, size_t preset_len,
                     bool *coolboost_enabled, const hardware_names *names,
                     daemon_runtime_state *runtime);

#endif
