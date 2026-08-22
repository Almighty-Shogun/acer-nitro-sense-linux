#ifndef ANS_DAEMON_CAPABILITIES_H
#define ANS_DAEMON_CAPABILITIES_H

#include "daemon/types.h"

/**
 * Reply with feature capability information.
 *
 * Capability output is a compact summary of which Acer Sense parity features
 * are available for the current model and Linux environment.
 */
void reply_capabilities(int client, const struct ans_config* cfg, const daemon_runtime_state* runtime);

#endif
