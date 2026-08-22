#include "client/doctor_util.h"

#include "util/file.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void doctor_print_section(const char *title)
{
    printf("\n## %s\n", title);
}

void doctor_print_file_value(const char *label, const char *path)
{
    char *text = read_text_file(path, 4096);

    if (!text) {
        printf("%s=unavailable path=%s error=%s\n", label, path, strerror(errno));
        return;
    }

    char *newline = strchr(text, '\n');
    if (newline)
        *newline = '\0';

    printf("%s=%s\n", label, text[0] ? text : "empty");
    free(text);
}
