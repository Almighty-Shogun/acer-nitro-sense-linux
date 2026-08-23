#include "hardware/names_internal.h"

#include "util/string.h"
#include "util/process.h"

#include <stdio.h>
#include <string.h>

/**
 * Extract a quoted field from one `lspci -Dmm` row.
 *
 * Machine-readable lspci output stores each visible value inside quotes. The
 * caller asks for a zero-based field and receives only the unquoted span.
 */
static bool extract_lspci_field(const char* line, const int index, char* out, const size_t out_len)
{
    int current = -1;

    for (const char* p = line; *p; p++)
    {
        if (*p != '"') continue;

        const char* start = ++p;
        const char* end = strchr(start, '"');

        if (!end)
            return false;

        current++;

        if (current == index)
        {
            string_copy_span(out, out_len, start, (size_t)(end - start));

            return true;
        }

        p = end;
    }

    return false;
}

/**
 * Parse the fields needed to identify one PCI display device.
 *
 * Rows that do not contain the expected class, vendor, and device fields are
 * ignored because they cannot produce a reliable user-facing GPU name.
 */
static bool parse_lspci_gpu_fields(
    const char* line,
    char* class,
    const size_t class_len,
    char* vendor,
    const size_t vendor_len,
    char* device,
    const size_t device_len
)
{
    return extract_lspci_field(line, 0, class, class_len)
           && extract_lspci_field(line, 1, vendor, vendor_len)
           && extract_lspci_field(line, 2, device, device_len);
}

/**
 * Return whether an lspci class represents a GPU/display controller.
 *
 * Acer Nitro systems can expose the discrete GPU as VGA, 3D, or Display
 * depending on driver and power-management state.
 */
static bool is_gpu_class(const char* class)
{
    return string_contains_case(class, "vga")
           || string_contains_case(class, "3d")
           || string_contains_case(class, "display");
}

/**
 * Join vendor and model text into the display name buffer.
 *
 * The name is only used in status output and desktop integration, so truncation
 * is acceptable as long as the string remains valid and readable.
 */
static void format_name_pair(char* out, const size_t out_len, const char* left, const char* right)
{
    if (out_len == 0) return;

    snprintf(out, out_len, "%s %s", left, right);
}

/**
 * Build a concise GPU name from lspci vendor and device fields.
 *
 * lspci often returns a long marketing/device string with a bracketed model.
 * Prefer that bracketed model when present and normalize NVIDIA capitalization.
 */
static void simplify_gpu_name(const char* vendor, const char* device, char* out, const size_t out_len)
{
    const char* brand = string_contains_case(vendor, "nvidia") ? "NVIDIA" : vendor;

    const char* open = strchr(device, '[');
    const char* close = open ? strchr(open, ']') : NULL;

    if (open && close && close > open + 1)
    {
        char model[128];

        string_copy_span(model, sizeof(model), open + 1, (size_t)(close - open - 1));
        format_name_pair(out, out_len, brand, model);
    }
    else
    {
        format_name_pair(out, out_len, brand, device);
    }

    trim_ascii(out);
}

/**
 * Load the preferred GPU display name.
 *
 * Discrete NVIDIA devices win because they are the component paired with the
 * GPU fan. If no NVIDIA display controller is found, the first display-class
 * PCI device becomes the fallback.
 */
void load_gpu_name(char* out, const size_t out_len)
{
    const char* const argv[] = {"lspci", "-Dmm", NULL};

    pid_t pid;
    FILE* lspci = process_open_stdout("lspci", argv, &pid);

    char line[512];
    char fallback[160] = "GPU";

    string_copy(out, out_len, "GPU");

    if (!lspci) return;

    while (fgets(line, sizeof(line), lspci))
    {
        char class[96], vendor[128], device[160], name[160];

        const bool is_gpu_fields = parse_lspci_gpu_fields(
            line,
            class,
            sizeof(class),
            vendor,
            sizeof(vendor),
            device,
            sizeof(device)
        );

        if (!is_gpu_fields || !is_gpu_class(class)) continue;

        simplify_gpu_name(vendor, device, name, sizeof(name));

        const bool is_nvidia = string_contains_case(vendor, "nvidia");

        if (strcmp(fallback, "GPU") == 0)
            string_copy(fallback, sizeof(fallback), name);

        if (is_nvidia)
        {
            string_copy(out, out_len, name);
            process_close_stdout(lspci, pid);

            return;
        }
    }

    process_close_stdout(lspci, pid);
    string_copy(out, out_len, fallback);
}
