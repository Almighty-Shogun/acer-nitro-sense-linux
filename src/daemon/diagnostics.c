#include "daemon/diagnostics.h"

#include "ec/ec.h"
#include "sensors/sensors.h"

#include <stdio.h>

/**
 * Probe EC.
 *
 * Daemon helpers own process lifetime and runtime coordination. They keep
 * privileged EC access behind one service instead of spreading it into
 * clients.
 */
void probe_ec(struct ec_device* ec, const struct ans_config* cfg)
{
    printf("backend=%s\n", ec->name);

    for (int i = 0; i < cfg->fan_len; i++)
    {
        const struct fan_config* fan = &cfg->fans[i];
        const int raw = cfg->read_words ? ec_read_word(ec, fan->read_register) : ec_read_byte(ec, fan->read_register);

        const int temp = sensor_read_group_max_c(fan->sensor_group);

        printf(
            "%s rpm=%d temp=%d read_register=0x%02x write_register=0x%02x\n",
            fan->id,
            raw,
            temp,
            fan->read_register,
            fan->write_register
        );
    }
}
