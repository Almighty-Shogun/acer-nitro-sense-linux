#include "fan/control.h"

#include "ec/ec.h"
#include "fan/internal.h"

/**
 * Apply init writes.
 *
 * Fan control is the core cooling state machine. Helpers in this module
 * translate model curves, user requests, and safety policy into EC write
 * values.
 */
void apply_init_writes(struct ec_device* ec, const struct ans_config* cfg)
{
    for (int i = 0; i < cfg->init_write_len; i++)
        ec_write_byte(ec, cfg->init_writes[i].reg, cfg->init_writes[i].value);
}

/**
 * Apply reset writes.
 *
 * Fan control is the core cooling state machine. Helpers in this module
 * translate model curves, user requests, and safety policy into EC write
 * values.
 */
void apply_reset_writes(struct ec_device* ec, const struct ans_config* cfg)
{
    for (int i = cfg->fan_len - 1; i >= 0; i--)
        fan_write_percent_raw(ec, &cfg->fans[i], cfg->fans[i].reset_speed);

    for (int i = cfg->init_write_len - 1; i >= 0; i--)
        ec_write_byte(ec, cfg->init_writes[i].reg, cfg->init_writes[i].reset_value);
}
