#ifndef ANS_CONTROL_SOCKET_H
#define ANS_CONTROL_SOCKET_H

#include <stdbool.h>

/**
 * Create the daemon control socket.
 *
 * The socket is created in the runtime directory with service-group ownership
 * so non-root users can access permitted commands.
 */
int make_socket(void);

/**
 * Return whether a socket client may change daemon state.
 *
 * Only users in the service group may mutate daemon state. Read-only status
 * remains broadly useful while fan and platform writes stay permission-gated.
 */
bool client_can_control(int client);

#endif
