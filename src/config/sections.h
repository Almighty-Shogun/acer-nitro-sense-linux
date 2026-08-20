#ifndef ANS_CONFIG_SECTIONS_H
#define ANS_CONFIG_SECTIONS_H

#include "config/types.h"

int config_parse_allowed_dmi(const char *json, struct ans_config *cfg);
int config_parse_init_writes(const char *json, struct ans_config *cfg);
int config_parse_fans(const char *json, struct ans_config *cfg);
int config_parse_presets(const char *json, struct ans_config *cfg);
int config_parse_safety(const char *json, struct ans_config *cfg);
int config_parse_fan_modes(const char *json, struct ans_config *cfg);
int config_parse_platform_profiles(const char *json, struct ans_config *cfg);
int config_parse_power_source_profiles(const char *json, struct ans_config *cfg);
int config_parse_keyboard_backlight(const char *json, struct ans_config *cfg);

#endif
