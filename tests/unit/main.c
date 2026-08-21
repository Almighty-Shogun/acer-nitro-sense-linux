#include "unit/daemon.h"

#include <signal.h>
#include <stdbool.h>

volatile sig_atomic_t daemon_running = 1;
bool daemon_quiet_logs = false;
bool daemon_persist_control_state = true;

int main(void)
{
    return run_unit_tests();
}
