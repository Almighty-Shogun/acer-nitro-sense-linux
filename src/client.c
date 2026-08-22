#include "commands/client/dispatch.h"

/**
 * Start the executable entry point.
 *
 * The process should do setup, delegate to the real entry path, and return an
 * ordinary shell status without leaving partially initialized state behind.
 */
int main(const int argc, char **argv)
{
    return client_run(argc, argv);
}
