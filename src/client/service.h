#ifndef ANS_CLIENT_SERVICE_H
#define ANS_CLIENT_SERVICE_H

/**
 * Run a systemd action for the installed daemon service.
 *
 * Lifecycle commands use systemd when the daemon is not already reachable
 * through the control socket.
 */
int client_run_systemctl(const char* action);

/**
 * Validate the active model configuration through the daemon binary.
 *
 * This path mirrors the installed service config so users validate what the
 * daemon will actually load.
 */
int client_validate_model(void);

#endif
