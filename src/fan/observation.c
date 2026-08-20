#include "fan/observation.h"

#include "ec/ec.h"
#include "fan/safety.h"
#include "sensors/sensors.h"

#include <stdio.h>
#include <string.h>

static void format_temperature_source(char *buf, const size_t buf_len,
                                      const char *kind, const char *group)
{
    snprintf(buf, buf_len, "%s:%s", kind, group && group[0] ? group : "unknown");
}

static int read_ec_temperature_c(struct ec_device *ec, const int reg)
{
    const int temp = reg >= 0 ? ec_read_byte(ec, reg) : -1;

    if (temp <= 0 || temp > 130)
        return -1;
    return temp;
}

const char *refresh_fan_observations(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     fan_state states[ANS_MAX_FANS])
{
    const char *forced_reason = "";

    for (int i = 0; i < cfg->fan_len; i++) {
        const struct fan_config *fan = &cfg->fans[i];
        const int raw = cfg->read_words ?
            ec_read_word(ec, fan->read_register) :
            ec_read_byte(ec, fan->read_register);
        const int write_value = ec_read_byte(ec, fan->write_register);
        const int ec_sensor_temp =
            read_ec_temperature_c(ec, fan->temperature_register);
        const int sensor_temp = ec_sensor_temp >= 0 ? ec_sensor_temp :
            sensor_read_group_max_c(fan->sensor_group);
        const bool uses_control_group = fan->control_sensor_group[0] &&
            strcmp(fan->control_sensor_group, fan->sensor_group) != 0;
        const int ec_control_sensor_temp =
            read_ec_temperature_c(ec, fan->control_temperature_register);
        const int control_sensor_temp = ec_control_sensor_temp >= 0 ?
            ec_control_sensor_temp :
            (uses_control_group ? sensor_read_group_max_c(fan->control_sensor_group) :
                                  sensor_temp);

        if (raw < 0) {
            states[i].rpm = -1;
            if (states[i].ec_read_failures < cfg->safety.max_ec_read_failures + 1)
                states[i].ec_read_failures++;
        } else {
            states[i].rpm = raw;
            states[i].ec_read_failures = 0;
        }

        states[i].write_value = write_value;
        states[i].sensor_temp_c = sensor_temp;
        states[i].control_sensor_temp_c = control_sensor_temp;
        if (sensor_temp >= 0) {
            char source[48];
            if (ec_sensor_temp >= 0)
                snprintf(source, sizeof(source), "component:ec:0x%02x",
                         fan->temperature_register);
            else
                format_temperature_source(source, sizeof(source), "component",
                                          fan->sensor_group);

            const int filtered_temp =
                fan_filtered_sensor_temp(fan->id, source, &states[i].temp_c,
                                         &states[i].pending_spike_temp_c,
                                         sensor_temp, states[i].temp_seeded,
                                         cfg->critical_temperature_c);

            states[i].temp_available = filtered_temp >= 0;
            if (filtered_temp >= 0) {
                states[i].temp_c = filtered_temp;
                states[i].temp_seeded = true;
            } else if (states[i].temp_c <= 0) {
                states[i].temp_c = -1;
            }
        } else {
            states[i].temp_available = false;
            if (states[i].temp_c <= 0)
                states[i].temp_c = -1;
        }

        if (control_sensor_temp >= 0) {
            char source[48];
            if (ec_control_sensor_temp >= 0)
                snprintf(source, sizeof(source), "control:ec:0x%02x",
                         fan->control_temperature_register);
            else
                format_temperature_source(source, sizeof(source), "control",
                                          uses_control_group ?
                                              fan->control_sensor_group :
                                              fan->sensor_group);

            const int filtered_control_temp =
                fan_filtered_sensor_temp(fan->id, source, &states[i].control_temp_c,
                                         &states[i].pending_control_spike_temp_c,
                                         control_sensor_temp,
                                         states[i].control_temp_seeded,
                                         cfg->critical_temperature_c);

            states[i].control_temp_available = filtered_control_temp >= 0;
            if (filtered_control_temp >= 0) {
                states[i].control_temp_c = filtered_control_temp;
                states[i].control_temp_seeded = true;
            } else if (states[i].control_temp_c <= 0) {
                states[i].control_temp_c = -1;
            }
        } else {
            states[i].control_temp_available = false;
            if (states[i].control_temp_c <= 0)
                states[i].control_temp_c = -1;
        }

        if (states[i].control_temp_available &&
            states[i].control_temp_c >= cfg->critical_temperature_c) {
            if (states[i].critical_temp_samples <
                cfg->safety.critical_consecutive_samples)
                states[i].critical_temp_samples++;
        } else if (states[i].control_temp_available &&
                   states[i].control_temp_c <=
                       cfg->safety.critical_release_temperature_c) {
            states[i].critical_temp_samples = 0;
        }

        if (states[i].ec_read_failures >= cfg->safety.max_ec_read_failures)
            forced_reason = "ec-read-failure";
        else if (states[i].critical_temp_samples >=
                     cfg->safety.critical_consecutive_samples &&
                 forced_reason[0] == '\0')
            forced_reason = "critical-temperature";
    }

    return forced_reason;
}
