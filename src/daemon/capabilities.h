#ifndef ANS_DAEMON_CAPABILITIES_H
#define ANS_DAEMON_CAPABILITIES_H

#include "daemon/types.h"

void reply_capabilities(int client, const struct ans_config *cfg,
                        const daemon_runtime_state *runtime);

#endif
