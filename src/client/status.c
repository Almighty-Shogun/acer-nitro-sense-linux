#include "client/status.h"

#include "client/transport.h"
#include "core/constants.h"
#include "util/file.h"
#include "util/string.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int convert_temp(int celsius, enum temp_unit unit)
{
    if (unit == TEMP_UNIT_FAHRENHEIT)
        return celsius * 9 / 5 + 32;

    return celsius;
}

static char unit_suffix(enum temp_unit unit)
{
    return unit == TEMP_UNIT_FAHRENHEIT ? 'F' : 'C';
}

static void print_status_token(const char *token, enum temp_unit unit)
{
    char *end;
    long temp;

    if (strncmp(token, "temp=", 5) == 0) {
        errno = 0;
        temp = strtol(token + 5, &end, 10);
        if (errno != 0 || end == token + 5 || *end != '\0') {
            fputs(token, stdout);
            return;
        }

        if (temp < 0)
            fputs("temp=--", stdout);
        else
            printf("temp=%d%c", convert_temp((int)temp, unit), unit_suffix(unit));

        return;
    }

    fputs(token, stdout);
}

static void print_status_token_span(const char *start, size_t len,
                                    enum temp_unit unit)
{
    char part[128];

    string_copy_span(part, sizeof(part), start, len);
    print_status_token(part, unit);
}

static void print_status_human(const char *status, enum temp_unit unit)
{
    const char *line = status;

    while (*line) {
        const char *line_end = strchr(line, '\n');
        const char *token = line;

        if (!line_end)
            line_end = line + strlen(line);

        while (token < line_end) {
            const char *token_end = token;

            while (token_end < line_end && *token_end != ' ' && *token_end != '\t')
                token_end++;

            if (token > line)
                putchar(' ');

            print_status_token_span(token, (size_t)(token_end - token), unit);

            token = token_end;
            while (token < line_end && (*token == ' ' || *token == '\t'))
                token++;
        }

        putchar('\n');

        line = *line_end == '\n' ? line_end + 1 : line_end;
    }
}

int client_print_status(enum temp_unit unit, bool json)
{
    char response[4096];

    if (json)
        return client_print_status_file();

    if (client_send_command_capture("status\n", true, response, sizeof(response)) == 0) {
        print_status_human(response, unit);
        return 0;
    }

    return client_print_status_file();
}

int client_print_coolboost_status(void)
{
    char response[256];
    char *status;
    char *available;
    char *enabled;

    if (client_send_command_capture("coolboost status\n", true, response,
                                    sizeof(response)) == 0) {
        fputs(response, stdout);
        return 0;
    }

    status = read_text_file(ANS_STATUS_PATH, 64 * 1024);
    if (!status) {
        perror(ANS_STATUS_PATH);
        return 1;
    }

    available = strstr(status, "\"coolboost_available\": true");
    enabled = strstr(status, "\"coolboost\": true");

    if (!available)
        printf("coolboost=unavailable\n");
    else
        printf("coolboost=%s\n", enabled ? "on" : "off");

    free(status);
    return 0;
}
