#ifndef ANS_EC_EC_H
#define ANS_EC_EC_H

#include "ec/types.h"

int ec_open_rw(const char *path, struct ec_device *ec);
void ec_close(struct ec_device *ec);
int ec_read_byte(struct ec_device *ec, int reg);
int ec_write_byte(struct ec_device *ec, int reg, int value);
int ec_read_word(struct ec_device *ec, int reg);

#endif
