#ifndef ANS_EC_TYPES_H
#define ANS_EC_TYPES_H

#include <stdint.h>

/**
 * EC access backend selected at runtime.
 *
 * Backends are ordered from kernel-mediated access to raw I/O and finally the
 * fake backend used by tests.
 */
enum ec_backend
{
    EC_BACKEND_NONE = 0,
    EC_BACKEND_EC_SYS,
    EC_BACKEND_ACPI_EC,
    EC_BACKEND_IO_PORTS,
    EC_BACKEND_FAKE,
};

/**
 * Open EC device state used by backend read/write operations.
 *
 * Real backends store a file descriptor and backend name; the fake backend
 * stores register bytes directly in memory.
 */
struct ec_device
{
    enum ec_backend backend;
    int fd;
    char name[32];
    uint8_t fake_regs[256];
};

#endif
