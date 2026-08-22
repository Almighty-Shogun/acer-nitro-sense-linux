#include "sensors/internal.h"

#include <dlfcn.h>
#include <stddef.h>
#include <string.h>

typedef int nvmlReturn_t;
typedef struct nvmlDevice_st* nvmlDevice_t;

typedef nvmlReturn_t (*nvml_init_fn)(void);
typedef nvmlReturn_t (*nvml_device_get_handle_by_index_fn)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t (*nvml_device_get_temperature_fn)(nvmlDevice_t, unsigned int, unsigned int*);

#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

/**
 * Load one NVML symbol into a typed function pointer slot.
 *
 * dlsym returns `void*`, while the callers store function pointers. Copying the
 * bytes avoids compiler-specific casts between object and function pointers.
 */
static void load_symbol(void* handle, const char* name, void* out, const size_t out_len)
{
    void* symbol = dlsym(handle, name);

    memset(out, 0, out_len);
    memcpy(out, &symbol, out_len < sizeof(symbol) ? out_len : sizeof(symbol));
}

/**
 * Load the preferred NVML init function.
 *
 * Newer drivers expose the v2 symbol; older drivers may only expose the legacy
 * name.
 */
static nvml_init_fn load_nvml_init_symbol(void* handle)
{
    nvml_init_fn fn;

    load_symbol(handle, "nvmlInit_v2", &fn, sizeof(fn));

    if (!fn)
        load_symbol(handle, "nvmlInit", &fn, sizeof(fn));

    return fn;
}

/**
 * Load the preferred NVML device lookup function.
 *
 * The v2 lookup symbol is preferred for current drivers, with the legacy symbol
 * kept as a compatibility fallback.
 */
static nvml_device_get_handle_by_index_fn load_nvml_device_lookup_symbol(void* handle)
{
    nvml_device_get_handle_by_index_fn fn;

    load_symbol(handle, "nvmlDeviceGetHandleByIndex_v2", &fn, sizeof(fn));

    if (!fn)
        load_symbol(handle, "nvmlDeviceGetHandleByIndex", &fn, sizeof(fn));

    return fn;
}

/**
 * Load the NVML temperature function.
 *
 * Temperature lookup has one stable symbol across the supported NVML versions.
 */
static nvml_device_get_temperature_fn load_nvml_temperature_symbol(void* handle)
{
    nvml_device_get_temperature_fn fn;

    load_symbol(handle, "nvmlDeviceGetTemperature", &fn, sizeof(fn));

    return fn;
}

/**
 * Return whether all required NVML symbols were loaded.
 *
 * Missing symbols are treated as sensor-unavailable instead of fatal, because
 * users can run this daemon on nouveau-only or non-NVIDIA systems.
 */
static bool nvml_symbols_available(
    const nvml_init_fn init,
    const nvml_device_get_handle_by_index_fn get_device,
    const nvml_device_get_temperature_fn get_temperature
)
{
    return init && get_device && get_temperature;
}

/**
 * Initialize NVML and resolve the first GPU device.
 *
 * The Acer Nitro models this daemon targets expose one discrete NVIDIA GPU, so
 * device index zero is the relevant temperature source.
 */
static bool initialize_nvml_device(
    const nvml_init_fn init,
    const nvml_device_get_handle_by_index_fn get_device,
    nvmlDevice_t* device
)
{
    return init() == NVML_SUCCESS && get_device(0, device) == NVML_SUCCESS;
}

/**
 * Return whether one NVML temperature read succeeded.
 *
 * Keeping the raw NVML return check here makes the public sensor helper read as
 * a simple unavailable-or-temperature result.
 */
static bool read_nvml_temperature_sample(
    const nvml_device_get_temperature_fn get_temperature,
    const nvmlDevice_t device,
    unsigned int* temp
)
{
    return get_temperature && get_temperature(device, NVML_TEMPERATURE_GPU, temp) == NVML_SUCCESS;
}

/**
 * Read one GPU temperature sample from NVML.
 *
 * A failed read returns `-1` so higher-level sensor code can fall back to sysfs
 * or report an unavailable GPU temperature.
 */
static int read_nvml_temperature_c(const nvml_device_get_temperature_fn get_temperature, const nvmlDevice_t device)
{
    unsigned int temp;

    if (!read_nvml_temperature_sample(get_temperature, device, &temp))
        return -1;

    return (int)temp;
}

/**
 * Read the NVIDIA GPU temperature through NVML.
 *
 * NVML is loaded lazily so the daemon does not require NVIDIA libraries on
 * systems that use nouveau, suspended GPUs, or non-NVIDIA hardware.
 */
int sensor_read_nvidia_ml_c(void)
{
    static void* handle;
    static bool initialized;

    static nvmlDevice_t device;
    static nvml_device_get_temperature_fn get_temperature;

    if (!initialized)
    {
        handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY | RTLD_LOCAL);

        if (!handle)
            return -1;

        const nvml_init_fn init = load_nvml_init_symbol(handle);
        const nvml_device_get_handle_by_index_fn get_device = load_nvml_device_lookup_symbol(handle);

        get_temperature = load_nvml_temperature_symbol(handle);

        const bool has_symbols = nvml_symbols_available(init, get_device, get_temperature);
        bool device_initialized = false;

        if (has_symbols)
            device_initialized = initialize_nvml_device(init, get_device, &device);

        if (!device_initialized)
        {
            dlclose(handle);

            handle = NULL;
            get_temperature = NULL;

            return -1;
        }

        initialized = true;
    }

    return read_nvml_temperature_c(get_temperature, device);
}
