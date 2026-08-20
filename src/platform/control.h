#ifndef ANS_PLATFORM_CONTROL_H
#define ANS_PLATFORM_CONTROL_H

#include "daemon/types.h"

extern const char FIRMWARE_AUTO_PRESET[];

bool firmware_auto_mode(bool auto_mode, const char *preset);
const char *control_mode(bool auto_mode, const char *preset);
const char *fan_mode_value_name(const struct fan_mode_config *fan_modes,
                                bool cpu, int value);
bool apply_fan_mode(struct ec_device *ec, const struct ans_config *cfg,
                    const char *mode);
bool read_fan_mode(struct ec_device *ec, const struct ans_config *cfg,
                   int *cpu_value, int *gpu_value);
bool apply_coolboost(struct ec_device *ec, const struct ans_config *cfg,
                     fan_state states[ANS_MAX_FANS], bool enabled);
bool apply_daemon_control_fan_mode(struct ec_device *ec, const struct ans_config *cfg);
bool apply_firmware_auto_fan_mode(struct ec_device *ec, const struct ans_config *cfg);
bool apply_platform_profile(struct ec_device *ec, const struct ans_config *cfg,
                            const char *profile);
const char *platform_profile_value_name(const struct ans_config *cfg, int value);
bool read_platform_profile(struct ec_device *ec, const struct ans_config *cfg,
                           int *value);
void apply_sensor_power_control(const struct ans_config *cfg, const char *control);

#endif
