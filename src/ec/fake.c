#include "ec/backend.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fake_set_word(struct ec_device *ec, int reg, int value)
{
    if (reg < 0 || reg + 1 >= (int)sizeof(ec->fake_regs))
        return;

    ec->fake_regs[reg] = (uint8_t)(value & 0xff);
    ec->fake_regs[reg + 1] = (uint8_t)((value >> 8) & 0xff);
}

static bool fake_ec_write_should_fail(const int reg)
{
    const char *fail_reg = getenv("ANS_FAKE_EC_WRITE_FAIL_REG");
    char *end;
    long parsed;

    if (!fail_reg || fail_reg[0] == '\0')
        return false;
    if (strcmp(fail_reg, "all") == 0)
        return true;

    parsed = strtol(fail_reg, &end, 0);
    return end != fail_reg && *end == '\0' && parsed == reg;
}

int ec_open_fake(struct ec_device *ec)
{
    ec->backend = EC_BACKEND_FAKE;
    ec->fd = -1;
    snprintf(ec->name, sizeof(ec->name), "fake");

    fake_set_word(ec, 0x13, 3000);
    fake_set_word(ec, 0x15, 2600);
    return 0;
}

int ec_fake_read_byte(struct ec_device *ec, const int reg)
{
    if (reg < 0 || reg >= (int)sizeof(ec->fake_regs)) {
        errno = ERANGE;
        return -1;
    }
    return ec->fake_regs[reg];
}

int ec_fake_write_byte(struct ec_device *ec, const int reg, const int value)
{
    if (reg < 0 || reg >= (int)sizeof(ec->fake_regs)) {
        errno = ERANGE;
        return -1;
    }
    if (fake_ec_write_should_fail(reg)) {
        errno = EIO;
        return -1;
    }
    ec->fake_regs[reg] = (uint8_t)clamp_int(value, 0, 255);
    return 0;
}
