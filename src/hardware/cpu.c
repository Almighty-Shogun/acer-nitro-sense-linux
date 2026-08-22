#include "hardware/names_internal.h"

#include "util/file.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Load the CPU model name from `/proc/cpuinfo`.
 *
 * Linux exposes the user-facing CPU model in the first `model name` entry. The
 * daemon uses it only for status output and falls back to `CPU` when the file
 * is unavailable or does not contain a model line.
 */
void load_cpu_name(char* out, const size_t out_len)
{
    char* cpu_info = read_text_file("/proc/cpuinfo", 256 * 1024);

    snprintf(out, out_len, "CPU");

    if (!cpu_info) return;

    char* line = strstr(cpu_info, "model name");

    if (line)
    {
        char* value = strchr(line, ':');
        char* end = strchr(line, '\n');

        if (value)
        {
            value++;

            while (*value == ' ' || *value == '\t')
                value++;

            if (!end)
                end = value + strlen(value);

            string_copy_span(out, out_len, value, (size_t)(end - value));
            trim_ascii(out);
        }
    }

    free(cpu_info);
}
