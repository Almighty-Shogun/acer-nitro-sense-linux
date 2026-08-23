#ifndef ANS_UNIT_STATUS_JSON_H
#define ANS_UNIT_STATUS_JSON_H

#include "daemon/types.h"

/**
 * Run status JSON formatting tests.
 *
 * These cases verify that the integration status file exposes stable fan and
 * feature fields.
 */
int unit_run_status_json(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

#endif
