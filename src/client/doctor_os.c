#include "client/doctor_util.h"

#include "util/file.h"
#include "util/string.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Read the operating system display name.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
static bool os_release_pretty_name(const char* text, char* out, const size_t out_len)
{
    const char* line = text;

    while (*line)
    {
        const char* line_end = strchr(line, '\n');

        if (!line_end)
            line_end = line + strlen(line);

        if (strncmp(line, "PRETTY_NAME=", 12) != 0)
        {
            line = *line_end == '\n' ? line_end + 1 : line_end;

            continue;
        }

        const char* value = line + 12;
        size_t len = (size_t)(line_end - value);

        if (len >= 2 && value[0] == '"' && value[len - 1] == '"')
        {
            value++;
            len -= 2;
        }

        string_copy_span(out, out_len, value, len);

        return true;
    }

    return false;
}

/**
 * Print OS pretty name.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
void doctor_print_os_pretty_name(void)
{
    char* text = read_text_file("/etc/os-release", 64 * 1024);

    char pretty_name[256];

    if (!text)
    {
        printf("os_pretty_name=unavailable path=/etc/os-release error=%s\n", strerror(errno));

        return;
    }

    if (os_release_pretty_name(text, pretty_name, sizeof(pretty_name)))
    {
        printf("os_pretty_name=%s\n", pretty_name);
    }
    else
    {
        printf("os_pretty_name=unknown\n");
    }

    free(text);
}
