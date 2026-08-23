#include "ec/backend.h"

#include "util/number.h"
#include "util/string.h"

#include <errno.h>
#include <stdint.h>
#include <sys/io.h>
#include <unistd.h>

#define EC_SC_PORT 0x66
#define EC_CMD_READ 0x80
#define EC_DATA_PORT 0x62
#define EC_CMD_WRITE 0x81
#define EC_STATUS_OBF 0x01
#define EC_STATUS_IBF 0x02

/**
 * Wait for clear.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
static int wait_status_clear(const uint8_t mask)
{
    for (int i = 0; i < 10000; i++)
    {
        if ((inb(EC_SC_PORT) & mask) == 0)
            return 0;

        usleep(10);
    }

    errno = ETIMEDOUT;

    return -1;
}

/**
 * Wait for set.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
static int wait_status_set(const uint8_t mask)
{
    for (int i = 0; i < 10000; i++)
    {
        if ((inb(EC_SC_PORT) & mask) != 0)
            return 0;

        usleep(10);
    }

    errno = ETIMEDOUT;

    return -1;
}

/**
 * Read I/O byte.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_io_read_byte(const int reg)
{
    if (wait_status_clear(EC_STATUS_IBF) < 0)
        return -1;

    outb(EC_CMD_READ, EC_SC_PORT);

    if (wait_status_clear(EC_STATUS_IBF) < 0)
        return -1;

    outb((uint8_t)reg, EC_DATA_PORT);

    if (wait_status_set(EC_STATUS_OBF) < 0)
        return -1;

    return inb(EC_DATA_PORT);
}

/**
 * Write I/O byte.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_io_write_byte(const int reg, const int value)
{
    if (wait_status_clear(EC_STATUS_IBF) < 0)
        return -1;

    outb(EC_CMD_WRITE, EC_SC_PORT);

    if (wait_status_clear(EC_STATUS_IBF) < 0)
        return -1;

    outb((uint8_t)reg, EC_DATA_PORT);

    if (wait_status_clear(EC_STATUS_IBF) < 0)
        return -1;

    outb((uint8_t)clamp_int(value, 0, 255), EC_DATA_PORT);

    return 0;
}

/**
 * Open I/O ports.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_open_io_ports(struct ec_device* ec)
{
#if defined(__i386__) || defined(__x86_64__)
    if (ioperm(EC_DATA_PORT, 1, 1) < 0)
        return -1;

    if (ioperm(EC_SC_PORT, 1, 1) < 0)
    {
        ioperm(EC_DATA_PORT, 1, 0);

        return -1;
    }

    ec->fd = -1;
    ec->backend = EC_BACKEND_IO_PORTS;

    string_copy(ec->name, sizeof(ec->name), "io-ports");

    return 0;
#else
    (void)ec;

    errno = ENOTSUP;

    return -1;
#endif
}

/**
 * Close I/O.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
void ec_io_close(void)
{
#if defined(__i386__) || defined(__x86_64__)
    ioperm(EC_DATA_PORT, 1, 0);
    ioperm(EC_SC_PORT, 1, 0);
#endif
}
