#ifndef ANS_DAEMON_TYPES_H
#define ANS_DAEMON_TYPES_H

#include "config/types.h"
#include "core/constants.h"
#include "ec/types.h"

#include <stdbool.h>
#include <signal.h>

typedef struct {
    int rpm;
    int temp_c;
    int sensor_temp_c;
    int control_temp_c;
    int control_sensor_temp_c;
    int pending_spike_temp_c;
    int pending_control_spike_temp_c;
    int write_value;
    bool temp_available;
    bool control_temp_available;
    bool temp_seeded;
    bool control_temp_seeded;
    int percent;
    int requested_percent;
    int critical_temp_samples;
    int ec_read_failures;
    int ec_write_failures;
    bool safety_active;
    char safety_reason[64];
} fan_state;

typedef struct {
    bool power_source_auto_apply;
    bool keyboard_backlight_timeout_enabled;
    bool keyboard_backlight_timed_off;
    int keyboard_backlight_timeout_seconds;
    int keyboard_backlight_restore_percent;
} daemon_runtime_state;

extern bool daemon_quiet_logs;
extern bool daemon_persist_control_state;
extern volatile sig_atomic_t daemon_running;

#endif
