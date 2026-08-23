#include "commands/client/dispatch.h"

/**
 * Start the client executable entry point.
 *
 * Argument handling and command routing belong to the dispatcher, so this
 * forwards its exit status unchanged and owns no process state of its own.
 */
int main(const int argc, char** argv)
{
    return client_run(argc, argv);
}
