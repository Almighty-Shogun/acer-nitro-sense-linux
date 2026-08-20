#include "sensors/internal.h"

#include <dlfcn.h>
#include <stddef.h>
#include <string.h>

typedef struct nvmlDevice_st *nvmlDevice_t;
typedef int nvmlReturn_t;

#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

static void load_symbol(void *handle, const char *name, void *out, const size_t out_len)
{
    void *symbol = dlsym(handle, name);

    memset(out, 0, out_len);
    memcpy(out, &symbol, out_len < sizeof(symbol) ? out_len : sizeof(symbol));
}

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

        load_symbol(handle, "nvmlInit_v2", &nvmlInit_fn, sizeof(nvmlInit_fn));
        if (!nvmlInit_fn)
            load_symbol(handle, "nvmlInit", &nvmlInit_fn, sizeof(nvmlInit_fn));

        load_symbol(handle, "nvmlDeviceGetHandleByIndex_v2",
                    &nvmlDeviceGetHandleByIndex_fn,
                    sizeof(nvmlDeviceGetHandleByIndex_fn));
        if (!nvmlDeviceGetHandleByIndex_fn)
            load_symbol(handle, "nvmlDeviceGetHandleByIndex",
                        &nvmlDeviceGetHandleByIndex_fn,
                        sizeof(nvmlDeviceGetHandleByIndex_fn));

        load_symbol(handle, "nvmlDeviceGetTemperature",
                    &nvmlDeviceGetTemperature_fn,
                    sizeof(nvmlDeviceGetTemperature_fn));

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
