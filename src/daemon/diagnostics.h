#ifndef ANS_DAEMON_DIAGNOSTICS_H
#define ANS_DAEMON_DIAGNOSTICS_H

#include "daemon/types.h"

void probe_ec(struct ec_device *ec, const struct ans_config *cfg);
void validate_model(struct ec_device *ec, const struct ans_config *cfg,
                    const char *config_path, bool force_model);
void print_ec_open_error(void);

#endif
