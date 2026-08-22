#ifndef ANS_CONFIG_FAN_TYPES_H
#define ANS_CONFIG_FAN_TYPES_H

#include "core/constants.h"

#include <stdbool.h>

struct threshold {
    int up;
    int down;
    int speed;
};

struct fan_config {
    char id[16];
    char name[64];
    char sensor_group[16];
    char control_sensor_group[16];
    char sensor_power_control[8];
    bool keep_awake;
    int read_register;
    int write_register;
    int temperature_register;
    int control_temperature_register;
    int read_min;
    int read_max;
    int write_min;
    int write_max;
    int reset_speed;
    int missing_temperature_speed_percent;
    struct threshold curve[ANS_MAX_THRESHOLDS];
    int curve_len;
};

struct ec_write_config {
    int reg;
    int value;
    int reset_value;
};

struct preset_config {
    char id[32];
    int cpu;
    int gpu;
};

#endif
