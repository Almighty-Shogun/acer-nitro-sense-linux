#include "ec/ec.h"
#include "ec/backend.h"

#include <errno.h>
#include <string.h>

#define ACPI_EC_PATH "/dev/ec"

/**
 * Open read-write.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_open_rw(const char* path, struct ec_device* ec)
{
    memset(ec, 0, sizeof(*ec));

    ec->backend = EC_BACKEND_NONE;
    ec->fd = -1;

    if (strcmp(path, "fake") == 0 || strcmp(path, "fake:") == 0)
        return ec_open_fake(ec);

    if (ec_open_file_backend(path, EC_BACKEND_EC_SYS, "ec-sys", ec) == 0)
        return 0;

    if (ec_open_file_backend(ACPI_EC_PATH, EC_BACKEND_ACPI_EC, "acpi-ec", ec) == 0)
        return 0;

    return ec_open_io_ports(ec);
}

/**
 * Close EC.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
void ec_close(struct ec_device* ec)
{
    if ((ec->backend == EC_BACKEND_EC_SYS || ec->backend == EC_BACKEND_ACPI_EC) && ec->fd >= 0)
        ec_file_close(ec);

    if (ec->backend == EC_BACKEND_IO_PORTS)
        ec_io_close();

    ec->fd = -1;
    ec->backend = EC_BACKEND_NONE;
}

/**
 * Read byte.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_read_byte(struct ec_device* ec, const int reg)
{
    if (ec->backend == EC_BACKEND_EC_SYS || ec->backend == EC_BACKEND_ACPI_EC)
        return ec_file_read_byte(ec, reg);

    if (ec->backend == EC_BACKEND_IO_PORTS)
        return ec_io_read_byte(reg);

    if (ec->backend == EC_BACKEND_FAKE)
        return ec_fake_read_byte(ec, reg);

    errno = ENODEV;

    return -1;
}

/**
 * Write byte.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_write_byte(struct ec_device* ec, const int reg, const int value)
{
    if (ec->backend == EC_BACKEND_EC_SYS || ec->backend == EC_BACKEND_ACPI_EC)
        return ec_file_write_byte(ec, reg, value);

    if (ec->backend == EC_BACKEND_IO_PORTS)
        return ec_io_write_byte(reg, value);

    if (ec->backend == EC_BACKEND_FAKE)
        return ec_fake_write_byte(ec, reg, value);

    errno = ENODEV;

    return -1;
}

/**
 * Read word.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_read_word(struct ec_device* ec, const int reg)
{
    const int low = ec_read_byte(ec, reg);

    if (low < 0)
        return -1;

    const int high = ec_read_byte(ec, reg + 1);

    if (high < 0)
        return -1;

    return low | (high << 8);
}
