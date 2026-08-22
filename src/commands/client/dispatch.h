#ifndef ANS_CLIENT_DISPATCH_H
#define ANS_CLIENT_DISPATCH_H

/**
 * Handles one top-level client command.
 *
 * Every command receives the original argv slice so each handler can enforce
 * its own argument contract before contacting the daemon.
 */
typedef int (*client_command_handler)(int argc, char** argv);

/**
 * Maps a client command name to its handler.
 *
 * The dispatcher uses this table as the single source of truth for public CLI
 * commands.
 */
typedef struct {
    const char* name;
    client_command_handler handler;
} client_command;

/**
 * Dispatch the requested client command.
 *
 * The client entry point resolves aliases, validates the command token, and
 * delegates argument handling to the selected command module.
 */
int client_run(int argc, char** argv);

#endif
