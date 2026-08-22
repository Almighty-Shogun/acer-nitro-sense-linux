#ifndef ANS_EC_EC_H
#define ANS_EC_EC_H

#include "ec/types.h"

/**
 * Open an EC device for byte and word access.
 *
 * The implementation tries the configured backend path and records the backend
 * selected for later reads and writes.
 */
int ec_open_rw(const char *path, struct ec_device *ec);

/**
 * Close the active EC backend.
 *
 * Backends clean up their own file descriptors or raw I/O permissions through
 * this common wrapper.
 */
void ec_close(struct ec_device *ec);

/**
 * Read one EC byte.
 *
 * The wrapper dispatches to the selected backend and returns a negative value
 * when the read fails.
 */
int ec_read_byte(struct ec_device *ec, int reg);

/**
 * Write one EC byte.
 *
 * The wrapper dispatches to the selected backend and reports backend write
 * failures as a non-zero return.
 */
int ec_write_byte(struct ec_device *ec, int reg, int value);

/**
 * Read one EC word.
 *
 * Word reads are used by models whose RPM counters span two EC registers.
 */
int ec_read_word(struct ec_device *ec, int reg);

#endif
