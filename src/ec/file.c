#include "ec/backend.h"

#include "util/number.h"
#include "util/string.h"

#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

/**
 * Open backend.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_open_file_backend(const char* path, const enum ec_backend backend, const char* name, struct ec_device* ec)
{
    ec->fd = open(path, O_RDWR | O_CLOEXEC);

    if (ec->fd < 0)
        return -1;

    ec->backend = backend;

    string_copy(ec->name, sizeof(ec->name), name);

    return 0;
}

/**
 * Read byte.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_file_read_byte(const struct ec_device* ec, const int reg)
{
    ssize_t n;
    uint8_t value = 0;

    do
    {
        n = pread(ec->fd, &value, 1, reg);
    }
    while (n < 0 && errno == EINTR);

    if (n != 1)
        return -1;

    return value;
}

/**
 * Write byte.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
int ec_file_write_byte(const struct ec_device* ec, const int reg, const int value)
{
    ssize_t n;

    const uint8_t byte = (uint8_t)clamp_int(value, 0, 255);

    do
    {
        n = pwrite(ec->fd, &byte, 1, reg);
    }
    while (n < 0 && errno == EINTR);

    return n == 1 ? 0 : -1;
}

/**
 * Close EC file.
 *
 * EC access is hardware-sensitive, so backend helpers keep raw reads and
 * writes behind one small interface. Callers should not care whether the byte
 * came from acpi_ec, a file, or tests.
 */
void ec_file_close(const struct ec_device* ec)
{
    if (ec->fd >= 0)
        close(ec->fd);
}
