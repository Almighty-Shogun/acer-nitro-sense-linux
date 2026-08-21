#ifndef ANS_DAEMON_STATUS_JSON_H
#define ANS_DAEMON_STATUS_JSON_H

#include "daemon/types.h"
#include "hardware/hardware.h"

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

bool format_status_json(char *buf, size_t buf_len, const struct ans_config *cfg,
                        struct ec_device *ec,
                        const fan_state states[ANS_MAX_FANS], bool auto_mode,
                        const char *preset, bool coolboost_enabled,
                        const hardware_names *names,
                        const daemon_runtime_state *runtime, time_t now);

#endif
