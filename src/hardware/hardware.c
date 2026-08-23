#include "hardware/hardware.h"

#include "config/types.h"
#include "hardware/names_internal.h"

#include <string.h>

/**
 * Load user-facing hardware names for fan components.
 *
 * Status output and desktop integrations use these strings to show CPU/GPU
 * labels without making fan control depend on hardware-name discovery.
 */
void load_hardware_names(hardware_names* names)
{
    load_cpu_name(names->cpu, sizeof(names->cpu));
    load_gpu_name(names->gpu, sizeof(names->gpu));
}

/**
 * Resolve the display name for a fan component.
 *
 * Hardware naming is cosmetic but user-visible. These helpers normalize
 * system data without feeding it back into the safety or EC control path.
 */
const char* component_name_for_fan(const hardware_names* names, const struct fan_config* fan)
{
    if (strcmp(fan->id, "cpu") == 0)
        return names->cpu;

    if (strcmp(fan->id, "gpu") == 0)
        return names->gpu;

    return fan->name;
}
