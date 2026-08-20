#ifndef ANS_CONTROL_SOCKET_H
#define ANS_CONTROL_SOCKET_H

#include <stdbool.h>

int make_socket(void);
bool client_can_control(int client);

#endif
