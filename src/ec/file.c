#include "ec/backend.h"

#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int ec_open_file_backend(const char *path, const enum ec_backend backend,
                         const char *name, struct ec_device *ec)
{
    ec->fd = open(path, O_RDWR | O_CLOEXEC);
    if (ec->fd < 0)
        return -1;

    ec->backend = backend;
    snprintf(ec->name, sizeof(ec->name), "%s", name);
    return 0;
}

int ec_file_read_byte(struct ec_device *ec, const int reg)
{
    uint8_t value = 0;
    ssize_t n;

    do {
        n = pread(ec->fd, &value, 1, reg);
    } while (n < 0 && errno == EINTR);

    if (n != 1)
        return -1;
    return value;
}

int ec_file_write_byte(struct ec_device *ec, const int reg, const int value)
{
    const uint8_t byte = (uint8_t)clamp_int(value, 0, 255);
    ssize_t n;

    do {
        n = pwrite(ec->fd, &byte, 1, reg);
    } while (n < 0 && errno == EINTR);

    return n == 1 ? 0 : -1;
}

void ec_file_close(struct ec_device *ec)
{
    if (ec->fd >= 0)
        close(ec->fd);
}
