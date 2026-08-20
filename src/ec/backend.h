#ifndef ANS_EC_BACKEND_H
#define ANS_EC_BACKEND_H

#include "ans.h"

int ec_open_fake(struct ec_device *ec);
int ec_fake_read_byte(struct ec_device *ec, int reg);
int ec_fake_write_byte(struct ec_device *ec, int reg, int value);

int ec_open_file_backend(const char *path, enum ec_backend backend,
                         const char *name, struct ec_device *ec);
int ec_file_read_byte(struct ec_device *ec, int reg);
int ec_file_write_byte(struct ec_device *ec, int reg, int value);
void ec_file_close(struct ec_device *ec);

int ec_open_io_ports(struct ec_device *ec);
int ec_io_read_byte(int reg);
int ec_io_write_byte(int reg, int value);
void ec_io_close(void);

#endif
