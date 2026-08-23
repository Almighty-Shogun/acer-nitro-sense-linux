#ifndef ANS_CLIENT_FAN_COMMANDS_H
#define ANS_CLIENT_FAN_COMMANDS_H

/**
 * Dispatch raw EC debug commands.
 *
 * These commands are routed through the daemon so privileged EC access stays
 * behind the control socket.
 */
int client_handle_ec_command(int argc, char** argv);

/**
 * Dispatch preset application and preset status commands.
 *
 * Known local subcommands are handled by the client; other names are forwarded
 * as model preset names.
 */
int client_handle_preset_command(int argc, char** argv);

/**
 * Dispatch manual fan percentage commands.
 *
 * The client validates fan id and percentage before asking the daemon to write
 * the requested speed.
 */
int client_handle_set_command(int argc, char** argv);

#endif
