#ifndef ANS_HARDWARE_H
#define ANS_HARDWARE_H

#include "ans.h"

typedef struct {
    char cpu[160];
    char gpu[160];
} hardware_names;

const char *load_dmi_model(void);
bool dmi_allowed(const struct ans_config *cfg, const char *dmi);
void load_hardware_names(hardware_names *names);
const char *component_name_for_fan(const hardware_names *names,
                                   const struct fan_config *fan);

#endif
