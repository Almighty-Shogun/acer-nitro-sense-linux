#include "daemon/diagnostics.h"

#include "util/file.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Return whether kernel lockdown blocks raw EC I/O.
 *
 * Daemon helpers own process lifetime and runtime coordination. They keep
 * privileged EC access behind one service instead of spreading it into
 * clients.
 */
static bool lockdown_blocks_raw_io(void)
{
    char* lockdown = read_text_file("/sys/kernel/security/lockdown", 256);

    if (!lockdown)
        return false;

    const bool blocked = strstr(lockdown, "[integrity]") || strstr(lockdown, "[confidentiality]");

    free(lockdown);

    return blocked;
}

/**
 * Print open error.
 *
 * Daemon helpers own process lifetime and runtime coordination. They keep
 * privileged EC access behind one service instead of spreading it into
 * clients.
 */
void print_ec_open_error(void)
{
    const int saved_errno = errno;

    errno = saved_errno;

    perror("EC backend");
    fprintf(stderr, "EC access failed. Tried ec_sys, acpi_ec (/dev/ec), then direct EC I/O ports.\n");

    if (saved_errno == EPERM && lockdown_blocks_raw_io())
    {
        fprintf(stderr, "Kernel lockdown is active and blocks direct EC I/O even as root.\n");
        fprintf(stderr, "This kernel also needs CONFIG_ACPI_EC_DEBUGFS for the ec_sys backend.\n");

        return;
    }

    fprintf(stderr, "Run as root and make sure the kernel allows CAP_SYS_RAWIO/ioperm.\n");
}
