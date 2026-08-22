#include "platform/control.h"

#include "sensors/sensors.h"

#include <stdio.h>
#include <string.h>

/**
 * Apply sensor power control.
 *
 * Platform controls mirror Acer Sense features that are actually reachable on
 * Linux. Unsupported firmware surfaces should report clearly instead of
 * pretending to work.
 */
void apply_sensor_power_control(const struct ans_config* cfg, const char* control)
{
    for (int i = 0; i < cfg->fan_len; i++)
    {
        const struct fan_config* fan = &cfg->fans[i];
        const char* target = "";

        if (strcmp(control, "auto") == 0 && fan->sensor_power_control[0])
        {
            target = "auto";
        }
        else if (fan->sensor_power_control[0])
        {
            target = fan->sensor_power_control;
        }
        else if (fan->keep_awake)
        {
            target = control;
        }

        if (!target[0]) continue;

        const int changed = sensor_set_group_power_control(fan->sensor_group, target);

        if (changed <= 0)
        {
            fprintf(stderr, "warning: failed to set %s sensor power control to %s\n", fan->sensor_group, target);
        }
        else
        {
            fprintf(
                stderr,
                "set %s sensor power control to %s (%d device%s)\n",
                fan->sensor_group,
                target,
                changed,
                changed == 1 ? "" : "s"
            );
        }
    }
}
