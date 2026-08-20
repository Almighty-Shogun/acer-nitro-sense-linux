#include "platform/control.h"

#include "fan/control.h"

#include <string.h>

static bool fan_mode_values(const struct fan_mode_config *fan_modes,
                            const char *mode, int *cpu_value,
                            int *gpu_value)
{
    if (strcmp(mode, "auto") == 0) {
        *cpu_value = fan_modes->cpu_auto_value;
        *gpu_value = fan_modes->gpu_auto_value;
        return true;
    }

    if (strcmp(mode, "manual") == 0) {
        *cpu_value = fan_modes->cpu_manual_value;
        *gpu_value = fan_modes->gpu_manual_value;
        return true;
    }

    if (strcmp(mode, "turbo") == 0) {
        *cpu_value = fan_modes->cpu_turbo_value;
        *gpu_value = fan_modes->gpu_turbo_value;
        return true;
    }

    return false;
}

const char *fan_mode_value_name(const struct fan_mode_config *fan_modes,
                                const bool cpu, const int value)
{
    if (cpu) {
        if (value == fan_modes->cpu_auto_value)
            return "auto";
        if (value == fan_modes->cpu_manual_value)
            return "manual";
        if (value == fan_modes->cpu_turbo_value)
            return "turbo";
    } else {
        if (value == fan_modes->gpu_auto_value)
            return "auto";
        if (value == fan_modes->gpu_manual_value)
            return "manual";
        if (value == fan_modes->gpu_turbo_value)
            return "turbo";
    }

    return "unknown";
}

bool apply_fan_mode(struct ec_device *ec, const struct ans_config *cfg,
                    const char *mode)
{
    int cpu_value;
    int gpu_value;

    if (!cfg->fan_modes.available ||
        !fan_mode_values(&cfg->fan_modes, mode, &cpu_value, &gpu_value))
        return false;

    return ec_write_byte(ec, cfg->fan_modes.cpu_reg, cpu_value) == 0 &&
           ec_write_byte(ec, cfg->fan_modes.gpu_reg, gpu_value) == 0;
}

bool read_fan_mode(struct ec_device *ec, const struct ans_config *cfg,
                   int *cpu_value, int *gpu_value)
{
    if (!cfg->fan_modes.available)
        return false;

    *cpu_value = ec_read_byte(ec, cfg->fan_modes.cpu_reg);
    *gpu_value = ec_read_byte(ec, cfg->fan_modes.gpu_reg);

    return *cpu_value >= 0 && *gpu_value >= 0;
}

bool apply_coolboost(struct ec_device *ec, const struct ans_config *cfg,
                     fan_state states[ANS_MAX_FANS], const bool enabled)
{
    if (!cfg->fan_modes.available)
        return false;

    if (enabled)
        return apply_fan_mode(ec, cfg, "turbo");

    if (!apply_fan_mode(ec, cfg, "manual"))
        return false;

    apply_current_control_state(ec, cfg, states);
    return true;
}

bool apply_daemon_control_fan_mode(struct ec_device *ec,
                                   const struct ans_config *cfg)
{
    if (!cfg->fan_modes.available)
        return true;

    return apply_fan_mode(ec, cfg, "manual");
}

bool apply_firmware_auto_fan_mode(struct ec_device *ec,
                                  const struct ans_config *cfg)
{
    if (!cfg->fan_modes.available)
        return false;

    return apply_fan_mode(ec, cfg, "auto");
}
