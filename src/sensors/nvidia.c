#include "sensors/internal.h"

#include <dlfcn.h>
#include <stddef.h>

typedef struct nvmlDevice_st *nvmlDevice_t;
typedef int nvmlReturn_t;

#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

int sensor_read_nvidia_ml_c(void)
{
    static void *handle;
    static bool initialized;
    static nvmlDevice_t device;
    static nvmlReturn_t (*nvmlDeviceGetTemperature_fn)(nvmlDevice_t, unsigned int, unsigned int *);
    nvmlReturn_t (*nvmlInit_fn)(void);
    nvmlReturn_t (*nvmlDeviceGetHandleByIndex_fn)(unsigned int, nvmlDevice_t *);
    unsigned int temp;

    if (!initialized) {
        handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY | RTLD_LOCAL);
        if (!handle)
            return -1;

        nvmlInit_fn = (nvmlReturn_t (*)(void))dlsym(handle, "nvmlInit_v2");
        if (!nvmlInit_fn)
            nvmlInit_fn = (nvmlReturn_t (*)(void))dlsym(handle, "nvmlInit");

        nvmlDeviceGetHandleByIndex_fn =
            (nvmlReturn_t (*)(unsigned int, nvmlDevice_t *))dlsym(handle,
                                                                  "nvmlDeviceGetHandleByIndex_v2");
        if (!nvmlDeviceGetHandleByIndex_fn)
            nvmlDeviceGetHandleByIndex_fn =
                (nvmlReturn_t (*)(unsigned int, nvmlDevice_t *))dlsym(handle,
                                                                      "nvmlDeviceGetHandleByIndex");

        nvmlDeviceGetTemperature_fn =
            (nvmlReturn_t (*)(nvmlDevice_t, unsigned int, unsigned int *))dlsym(
                handle, "nvmlDeviceGetTemperature");

        if (!nvmlInit_fn || !nvmlDeviceGetHandleByIndex_fn || !nvmlDeviceGetTemperature_fn ||
            nvmlInit_fn() != NVML_SUCCESS ||
            nvmlDeviceGetHandleByIndex_fn(0, &device) != NVML_SUCCESS) {
            dlclose(handle);
            handle = NULL;
            return -1;
        }

        initialized = true;
    }

    if (!nvmlDeviceGetTemperature_fn ||
        nvmlDeviceGetTemperature_fn(device, NVML_TEMPERATURE_GPU, &temp) != NVML_SUCCESS)
        return -1;

    return (int)temp;
}
