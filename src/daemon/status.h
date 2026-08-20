#ifndef ANS_DAEMON_STATUS_H
#define ANS_DAEMON_STATUS_H

#include "daemon/types.h"
#include "hardware/hardware.h"

void write_status(const struct ans_config *cfg, struct ec_device *ec,
                  const fan_state states[ANS_MAX_FANS], bool auto_mode,
                  const char *preset, bool coolboost_enabled,
                  const hardware_names *names,
                  const daemon_runtime_state *runtime);
void write_temperature_cache(const struct ans_config *cfg,
                             const fan_state states[ANS_MAX_FANS]);
void reply_status(int client, const struct ans_config *cfg,
                  const fan_state states[ANS_MAX_FANS], bool auto_mode,
                  const char *preset, bool coolboost_enabled);
void reply_preset_show(int client, const struct ans_config *cfg,
                       const fan_state states[ANS_MAX_FANS],
                       bool auto_mode, const char *preset);
void reply_presets(int client, const struct ans_config *cfg);
void reply_ec_read(int client, struct ec_device *ec, int reg);
void reply_ec_dump(int client, struct ec_device *ec, int start, int end);

#endif
