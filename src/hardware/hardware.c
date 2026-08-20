#include "hardware/hardware.h"
#include "hardware/names_internal.h"

#include <string.h>

void load_hardware_names(hardware_names *names)
{
    load_cpu_name(names->cpu, sizeof(names->cpu));
    load_gpu_name(names->gpu, sizeof(names->gpu));
}

const char *component_name_for_fan(const hardware_names *names,
                                   const struct fan_config *fan)
{
    if (strcmp(fan->id, "cpu") == 0)
        return names->cpu;
    if (strcmp(fan->id, "gpu") == 0)
        return names->gpu;
    return fan->name;
}
