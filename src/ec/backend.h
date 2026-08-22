#ifndef ANS_EC_BACKEND_H
#define ANS_EC_BACKEND_H

#include "ec/types.h"

/**
 * Open the in-memory fake EC backend.
 *
 * Tests use the fake backend to exercise fan logic without privileged hardware
 * access.
 */
int ec_open_fake(struct ec_device* ec);

/**
 * Read one byte from the fake EC register array.
 *
 * The fake backend mirrors the real backend API so production code does not
 * need test-specific branches.
 */
int ec_fake_read_byte(const struct ec_device* ec, int reg);

/**
 * Write one byte to the fake EC register array.
 *
 * The value is stored directly in the fake device so tests can assert later EC
 * state.
 */
int ec_fake_write_byte(struct ec_device* ec, int reg, int value);

/**
 * Open a file-backed EC backend.
 *
 * This covers kernel interfaces such as `/dev/ec` and debugfs EC access that
 * expose register reads through a file descriptor.
 */
int ec_open_file_backend(const char* path, enum ec_backend backend, const char* name, struct ec_device* ec);

/**
 * Read one byte from a file-backed EC backend.
 *
 * The backend seeks to the requested register before reading so callers can
 * access arbitrary EC offsets.
 */
int ec_file_read_byte(const struct ec_device* ec, int reg);

/**
 * Write one byte through a file-backed EC backend.
 *
 * The backend seeks to the requested register before writing the new value.
 */
int ec_file_write_byte(const struct ec_device* ec, int reg, int value);

/**
 * Close a file-backed EC backend.
 *
 * The file descriptor is released and the device state is reset for reuse.
 */
void ec_file_close(const struct ec_device* ec);

/**
 * Open raw EC I/O port access.
 *
 * This backend requires raw I/O permission and is used only when kernel EC
 * interfaces are unavailable.
 */
int ec_open_io_ports(struct ec_device* ec);

/**
 * Read one byte through raw EC I/O ports.
 *
 * Port access follows the legacy EC command/data protocol and must only be
 * used after `ec_open_io_ports()` succeeds.
 */
int ec_io_read_byte(int reg);

/**
 * Write one byte through raw EC I/O ports.
 *
 * Writes use the same legacy EC command/data protocol as raw reads.
 */
int ec_io_write_byte(int reg, int value);

/**
 * Release raw EC I/O port access.
 *
 * This drops the process permissions acquired by the raw I/O backend.
 */
void ec_io_close(void);

#endif
