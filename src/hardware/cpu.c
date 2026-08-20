#include "hardware/names_internal.h"

#include "ans.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_cpu_name(char *out, const size_t out_len)
{
    char *cpu_info = read_text_file("/proc/cpuinfo", 256 * 1024);

    snprintf(out, out_len, "CPU");

    if (!cpu_info)
        return;

    char *line = strstr(cpu_info, "model name");

    if (line) {
        char *value = strchr(line, ':');
        char *end = strchr(line, '\n');

        if (value) {
            value++;
            while (*value == ' ' || *value == '\t')
                value++;

            if (!end)
                end = value + strlen(value);

            size_t len = (size_t)(end - value);

            if (len >= out_len)
                len = out_len - 1;

            memcpy(out, value, len);
            out[len] = '\0';
            trim_ascii(out);
        }
    }

    free(cpu_info);
}
