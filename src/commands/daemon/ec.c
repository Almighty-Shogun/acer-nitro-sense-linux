#include "commands/daemon/ec.h"

#include "daemon/status.h"
#include "control/protocol.h"
#include "commands/parser/parser.h"

#include <string.h>

/**
 * Handles one daemon EC debug command.
 *
 * EC debug handlers parse their own arguments because read and dump expose
 * different bounded register access patterns.
 */
typedef bool (*ec_command_handler)(int client, struct ec_device* ec, const char* cmd);

/**
 * Maps an EC debug command name to its daemon handler.
 *
 * Only explicit EC debug commands are routed here; all permission checks are
 * performed by the daemon registry before this table is reached.
 */
typedef struct
{
    const char* name;
    ec_command_handler handler;
} ec_command;

/**
 * Read one EC register for diagnostics.
 *
 * The registry has already checked permissions, so this handler only validates
 * the register argument and returns the formatted EC value.
 */
static bool handle_ec_read_command(const int client, struct ec_device* ec, const char* cmd)
{
    int ec_start;

    if (!parse_ec_read_command(cmd, &ec_start))
    {
        control_reply(client, "error usage: ec read REG\n");

        return true;
    }

    reply_ec_read(client, ec, ec_start);

    return true;
}

/**
 * Dump a bounded EC register range for diagnostics.
 *
 * Dumps are range-limited by the parser so debug output remains predictable
 * and cannot accidentally read the whole EC space.
 */
static bool handle_ec_dump_command(const int client, struct ec_device* ec, const char* cmd)
{
    int ec_end;
    int ec_start;

    if (!parse_ec_dump_command(cmd, &ec_start, &ec_end))
    {
        control_reply(client, "error usage: ec dump START END\n");

        return true;
    }

    reply_ec_dump(client, ec, ec_start, ec_end);

    return true;
}

/**
 * Supported daemon EC debug commands.
 *
 * The daemon exposes flat protocol commands even though the client groups them
 * under `ec`, so routing stays explicit here.
 */
static const ec_command EC_COMMANDS[] = {
    {.name = "ec-read", .handler = handle_ec_read_command},
    {.name = "ec-dump", .handler = handle_ec_dump_command},
};

/**
 * Find an EC command entry.
 *
 * EC command routing is table-driven so read and dump behavior stay explicit.
 */
static const ec_command* find_ec_command(const char* name)
{
    for (size_t i = 0; i < COMMAND_ARRAY_LEN(EC_COMMANDS); i++)
    {
        if (strcmp(name, EC_COMMANDS[i].name) == 0)
            return &EC_COMMANDS[i];
    }

    return NULL;
}

/**
 * Dispatch a daemon EC debug command.
 *
 * Unknown EC command names are reported as daemon protocol errors rather than
 * falling through into another feature command group.
 */
bool handle_ec_command(const int client, struct ec_device* ec, const char* cmd)
{
    char command[32];

    if (!command_first_token(cmd, command, sizeof(command)))
    {
        control_reply(client, "error unknown command\n");

        return true;
    }

    const ec_command* entry = find_ec_command(command);

    if (entry)
        return entry->handler(client, ec, cmd);

    control_reply(client, "error unknown command\n");

    return true;
}
