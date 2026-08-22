#ifndef ANS_DAEMON_STATUS_JSON_INTERNAL_H
#define ANS_DAEMON_STATUS_JSON_INTERNAL_H

#include "daemon/types.h"
#include "hardware/hardware.h"
#include "util/format.h"

void append_fan_status_json(text_buffer *out, const struct ans_config *cfg,
                            const fan_state states[ANS_MAX_FANS],
                            const hardware_names *names, bool firmware_mode,
                            int index);

#endif
