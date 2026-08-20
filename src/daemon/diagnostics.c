#include "daemon/diagnostics.h"

#include "ec/ec.h"
#include "hardware/hardware.h"
#include "platform/control.h"
#include "platform/power_source.h"
#include "sensors/sensors.h"
#include "util/file.h"
#include "util/string.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void probe_ec(struct ec_device *ec, const struct ans_config *cfg)
{
    printf("backend=%s\n", ec->name);

    for (int i = 0; i < cfg->fan_len; i++) {
        const struct fan_config *fan = &cfg->fans[i];
        const int raw = cfg->read_words ? ec_read_word(ec, fan->read_register) :
                                          ec_read_byte(ec, fan->read_register);
        const int temp = sensor_read_group_max_c(fan->sensor_group);

        printf("%s rpm=%d temp=%d read_register=0x%02x write_register=0x%02x\n",
               fan->id, raw, temp, fan->read_register, fan->write_register);
    }
}

void validate_model(struct ec_device *ec, const struct ans_config *cfg,
                    const char *config_path, const bool force_model)
{
    const char *dmi = load_dmi_model();
    const bool allowed = force_model || dmi_allowed(cfg, dmi);

    printf("config=%s\n", config_path);
    printf("model=%s\n", cfg->model);
    printf("dmi=%s\n", dmi[0] ? dmi : "unknown");
    printf("dmi_allowed=%s\n", allowed ? "true" : "false");
    printf("backend=%s\n", ec->name);
    printf("fans=%d presets=%d poll_interval_ms=%d critical_temperature_c=%d\n",
           cfg->fan_len, cfg->preset_len, cfg->poll_interval_ms,
           cfg->critical_temperature_c);
    printf("safety min_speed_percent=%d min_speed_temperature_c=%d critical_speed_percent=%d critical_full_speed=%s critical_step_percent=%d critical_consecutive_samples=%d critical_release_temperature_c=%d auto_ramp_up_percent=%d auto_ramp_bypass_temperature_c=%d missing_temperature_speed_percent=%d max_ec_read_failures=%d max_ec_write_failures=%d\n",
           cfg->safety.min_speed_percent, cfg->safety.min_speed_temperature_c,
           cfg->safety.critical_speed_percent,
           cfg->safety.critical_full_speed ? "true" : "false",
           cfg->safety.critical_step_percent,
           cfg->safety.critical_consecutive_samples,
           cfg->safety.critical_release_temperature_c,
           cfg->safety.auto_ramp_up_percent,
           cfg->safety.auto_ramp_bypass_temperature_c,
           cfg->safety.missing_temperature_speed_percent,
           cfg->safety.max_ec_read_failures, cfg->safety.max_ec_write_failures);
    printf("coolboost available=%s backend=%s default_enabled=%s\n",
           cfg->fan_modes.available ? "true" : "false",
           cfg->fan_modes.available ? "fan-mode-turbo" : "unavailable",
           cfg->coolboost.default_enabled ? "true" : "false");
    if (cfg->fan_modes.available)
        printf("fan_modes available=true cpu_register=0x%02x gpu_register=0x%02x cpu_auto=0x%02x cpu_manual=0x%02x cpu_turbo=0x%02x gpu_auto=0x%02x gpu_manual=0x%02x gpu_turbo=0x%02x\n",
               cfg->fan_modes.cpu_reg, cfg->fan_modes.gpu_reg,
               cfg->fan_modes.cpu_auto_value, cfg->fan_modes.cpu_manual_value,
               cfg->fan_modes.cpu_turbo_value, cfg->fan_modes.gpu_auto_value,
               cfg->fan_modes.gpu_manual_value, cfg->fan_modes.gpu_turbo_value);
    else
        printf("fan_modes available=false\n");

    if (cfg->platform_profiles.available) {
        printf("platform_profiles available=true register=0x%02x default=%s profiles=%d\n",
               cfg->platform_profiles.reg,
               cfg->platform_profiles.default_profile[0] ?
                   cfg->platform_profiles.default_profile : "none",
               cfg->platform_profiles.profile_len);
        for (int i = 0; i < cfg->platform_profiles.profile_len; i++)
            printf("platform_profile=%s value=0x%02x\n",
                   cfg->platform_profiles.profiles[i].id,
                   cfg->platform_profiles.profiles[i].value);
    } else {
        printf("platform_profiles available=false\n");
    }

    printf("power_source_profiles available=%s auto_apply=%s ac=%s battery=%s current=%s\n",
           power_source_profile_policy_available(cfg) ? "true" : "false",
           cfg->power_source_profiles.auto_apply ? "true" : "false",
           cfg->power_source_profiles.ac_profile[0] ?
               cfg->power_source_profiles.ac_profile : "none",
           cfg->power_source_profiles.battery_profile[0] ?
               cfg->power_source_profiles.battery_profile : "none",
           power_source_name(read_power_source()));

    for (int i = 0; i < cfg->fan_len; i++) {
        const struct fan_config *fan = &cfg->fans[i];
        const int raw = cfg->read_words ? ec_read_word(ec, fan->read_register) :
                                          ec_read_byte(ec, fan->read_register);
        const int ec_temp = fan->temperature_register >= 0 ?
            ec_read_byte(ec, fan->temperature_register) : -1;
        const int ec_control_temp = fan->control_temperature_register >= 0 ?
            ec_read_byte(ec, fan->control_temperature_register) : -1;
        char temperature_register[16];
        char control_temperature_register[16];
        const int temp = ec_temp > 0 && ec_temp <= 130 ? ec_temp :
            sensor_read_group_max_c(fan->sensor_group);
        const int control_temp = ec_control_temp > 0 && ec_control_temp <= 130 ?
            ec_control_temp : fan->control_sensor_group[0] ?
            sensor_read_group_max_c(fan->control_sensor_group) : temp;

        if (fan->temperature_register >= 0)
            snprintf(temperature_register, sizeof(temperature_register), "0x%02x",
                     fan->temperature_register);
        else
            string_copy(temperature_register, sizeof(temperature_register), "none");
        if (fan->control_temperature_register >= 0)
            snprintf(control_temperature_register,
                     sizeof(control_temperature_register), "0x%02x",
                     fan->control_temperature_register);
        else
            string_copy(control_temperature_register,
                        sizeof(control_temperature_register), "none");

        printf("fan=%s name=\"%s\" sensor_group=%s control_sensor_group=%s rpm=%d temp=%d control_temp=%d read_register=0x%02x write_register=0x%02x temperature_register=%s control_temperature_register=%s reset_speed=%d missing_temperature_speed_percent=%d curve_points=%d keep_awake=%s sensor_power_control=%s\n",
               fan->id, fan->name, fan->sensor_group,
               fan->control_sensor_group[0] ? fan->control_sensor_group :
                   fan->sensor_group,
               raw, temp, control_temp, fan->read_register,
               fan->write_register, temperature_register,
               control_temperature_register,
               fan->reset_speed,
               fan->missing_temperature_speed_percent > 0 ?
                   fan->missing_temperature_speed_percent :
                   cfg->safety.missing_temperature_speed_percent,
               fan->curve_len,
               fan->keep_awake ? "true" : "false",
               fan->sensor_power_control[0] ? fan->sensor_power_control :
                   "default");
    }
}

static bool lockdown_blocks_raw_io(void)
{
    char *lockdown = read_text_file("/sys/kernel/security/lockdown", 256);

    if (!lockdown)
        return false;

    const bool blocked = strstr(lockdown, "[integrity]") ||
                         strstr(lockdown, "[confidentiality]");

    free(lockdown);

    return blocked;
}

void print_ec_open_error(void)
{
    const int saved_errno = errno;

    errno = saved_errno;

    perror("EC backend");
    fprintf(stderr,
            "EC access failed. Tried ec_sys, acpi_ec (/dev/ec), then direct EC I/O ports.\n");

    if (saved_errno == EPERM && lockdown_blocks_raw_io()) {
        fprintf(stderr,
                "Kernel lockdown is active and blocks direct EC I/O even as root.\n");
        fprintf(stderr,
                "This kernel also needs CONFIG_ACPI_EC_DEBUGFS for the ec_sys backend.\n");

        return;
    }

    fprintf(stderr,
            "Run as root and make sure the kernel allows CAP_SYS_RAWIO/ioperm.\n");
}
