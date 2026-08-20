#ifndef ANS_EC_TYPES_H
#define ANS_EC_TYPES_H

#include <stdint.h>

enum ec_backend {
    EC_BACKEND_NONE = 0,
    EC_BACKEND_EC_SYS,
    EC_BACKEND_ACPI_EC,
    EC_BACKEND_IO_PORTS,
    EC_BACKEND_FAKE,
};

struct ec_device {
    enum ec_backend backend;
    int fd;
    char name[32];
    uint8_t fake_regs[256];
};

#endif
