#include "sensors/internal.h"

#include <stdlib.h>
#include <string.h>

int sensor_read_fake_temp_c(const char *group)
{
    const char *env = NULL;
    char *end;
    long temp;

    if (strcmp(group, "max") == 0) {
        const int cpu_temp = sensor_read_fake_temp_c("cpu");
        const int gpu_temp = sensor_read_fake_temp_c("gpu");

        if (cpu_temp < 0)
            return gpu_temp;
        if (gpu_temp < 0)
            return cpu_temp;
        return cpu_temp > gpu_temp ? cpu_temp : gpu_temp;
    }

    if (strcmp(group, "cpu") == 0)
        env = getenv("ANS_FAKE_CPU_TEMP_C");
    else if (strcmp(group, "gpu") == 0)
        env = getenv("ANS_FAKE_GPU_TEMP_C");

    if (!env || !env[0])
        return -1;

    temp = strtol(env, &end, 10);
    if (end == env || *end != '\0' || temp < 0 || temp > 130)
        return -1;

    return (int)temp;
}
