#include "commands/ec.h"

#include "control/protocol.h"
#include "commands/parser.h"
#include "daemon/status.h"

#include <stdio.h>

bool handle_ec_command(const int client, struct ec_device *ec, const char *cmd)
{
    int ec_start;
    int ec_end;

    if (command_name_is(cmd, "ec-read")) {
        if (!parse_ec_read_command(cmd, &ec_start)) {
            control_reply(client, "error usage: ec read REG\n");
            return true;
        }

        reply_ec_read(client, ec, ec_start);
        return true;
    }

    if (command_name_is(cmd, "ec-dump")) {
        if (!parse_ec_dump_command(cmd, &ec_start, &ec_end)) {
            control_reply(client, "error usage: ec dump START END\n");
            return true;
        }

        reply_ec_dump(client, ec, ec_start, ec_end);
        return true;
    }

    return false;
}
