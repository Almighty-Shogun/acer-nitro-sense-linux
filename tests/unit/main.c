#include "unit/daemon/daemon.h"

#include <signal.h>
#include <stdbool.h>

// NOLINTNEXTLINE(misc-use-internal-linkage): test-owned definition for extern daemon modules.
volatile sig_atomic_t daemon_running = 1;

// NOLINTNEXTLINE(misc-use-internal-linkage): test-owned definition for extern daemon modules.
bool daemon_quiet_logs = false;

// NOLINTNEXTLINE(misc-use-internal-linkage): test-owned definition for extern daemon modules.
bool daemon_persist_control_state = true;

/**
 * Start the executable entry point.
 *
 * The process should do setup, delegate to the real entry path, and return an
 * ordinary shell status without leaving partially initialized state behind.
 */
int main(void)
{
    return run_unit_tests();
}
