#ifndef ANS_CONFIG_CONFIG_H
#define ANS_CONFIG_CONFIG_H

#include "config/types.h"

int config_load(const char *path, struct ans_config *cfg);
const struct preset_config *config_find_preset(const struct ans_config *cfg, const char *id);
const struct platform_profile_entry *config_find_platform_profile(const struct ans_config *cfg,
                                                                  const char *id);
const struct fan_config *config_find_fan(const struct ans_config *cfg, const char *id);

#endif
