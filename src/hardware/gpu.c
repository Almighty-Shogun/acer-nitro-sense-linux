#include "hardware/names_internal.h"

#include "util/string.h"

#include <stdio.h>
#include <string.h>

static bool extract_lspci_field(const char *line, const int index, char *out,
                                const size_t out_len)
{
    int current = -1;

    for (const char *p = line; *p; p++) {
        if (*p != '"')
            continue;

        const char *start = ++p;
        const char *end = strchr(start, '"');

        if (!end)
            return false;

        current++;

        if (current == index) {
            string_copy_span(out, out_len, start, (size_t)(end - start));

            return true;
        }
        p = end;
    }

    return false;
}

static void format_name_pair(char *out, const size_t out_len, const char *left,
                             const char *right)
{
    size_t used = 0;

    if (out_len == 0)
        return;

    out[0] = '\0';
    size_t left_len = strlen(left);

    if (left_len >= out_len)
        left_len = out_len - 1;

    memcpy(out, left, left_len);
    used = left_len;
    out[used] = '\0';

    if (used + 1 < out_len) {
        out[used++] = ' ';
        out[used] = '\0';
    }

    size_t right_len = strlen(right);

    if (right_len >= out_len - used)
        right_len = out_len - used - 1;

    memcpy(out + used, right, right_len);
    used += right_len;
    out[used] = '\0';
}

static void simplify_gpu_name(const char *vendor, const char *device, char *out,
                              const size_t out_len)
{
    const char *brand = string_contains_case(vendor, "nvidia") ? "NVIDIA" : vendor;
    const char *open = strchr(device, '[');
    const char *close = open ? strchr(open, ']') : NULL;

    if (open && close && close > open + 1) {
        char model[128];

        string_copy_span(model, sizeof(model), open + 1,
                         (size_t)(close - open - 1));
        format_name_pair(out, out_len, brand, model);
    } else {
        format_name_pair(out, out_len, brand, device);
    }

    trim_ascii(out);
}

void load_gpu_name(char *out, const size_t out_len)
{
    FILE *lspci = popen("lspci -Dmm", "r");
    char fallback[160] = "GPU";
    char line[512];

    string_copy(out, out_len, "GPU");

    if (!lspci)
        return;

    while (fgets(line, sizeof(line), lspci)) {
        char class[96];
        char vendor[128];
        char device[160];
        char name[160];

        if (!extract_lspci_field(line, 0, class, sizeof(class)) ||
            !extract_lspci_field(line, 1, vendor, sizeof(vendor)) ||
            !extract_lspci_field(line, 2, device, sizeof(device)))
            continue;

        const bool is_gpu = string_contains_case(class, "vga") ||
            string_contains_case(class, "3d") ||
            string_contains_case(class, "display");

        if (!is_gpu)
            continue;

        simplify_gpu_name(vendor, device, name, sizeof(name));
        const bool is_nvidia = string_contains_case(vendor, "nvidia");

        if (strcmp(fallback, "GPU") == 0)
            string_copy(fallback, sizeof(fallback), name);

        if (is_nvidia) {
            string_copy(out, out_len, name);
            pclose(lspci);

            return;
        }
    }

    pclose(lspci);
    string_copy(out, out_len, fallback);
}
