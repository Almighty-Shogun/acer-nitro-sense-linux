#include "client/status.h"

#include "util/file.h"
#include "util/string.h"
#include "core/constants.h"
#include "client/transport.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Convert temp.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
static int convert_temp(const int celsius, const enum temp_unit unit)
{
    if (unit == TEMP_UNIT_FAHRENHEIT)
        return celsius * 9 / 5 + 32;

    return celsius;
}

/**
 * Choose the temperature unit suffix.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
static char unit_suffix(const enum temp_unit unit)
{
    return unit == TEMP_UNIT_FAHRENHEIT ? 'F' : 'C';
}

/**
 * Print token.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
static void print_status_token(const char* token, const enum temp_unit unit)
{
    char* end;

    if (strncmp(token, "temp=", 5) == 0)
    {
        errno = 0;
        const long temp = strtol(token + 5, &end, 10);

        if (errno != 0 || end == token + 5 || *end != '\0')
        {
            fputs(token, stdout);

            return;
        }

        if (temp < 0)
        {
            fputs("temp=--", stdout);
        }
        else
        {
            printf("temp=%d%c", convert_temp((int)temp, unit), unit_suffix(unit));
        }

        return;
    }

    fputs(token, stdout);
}

/**
 * Print token span.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
static void print_status_token_span(const char* start, const size_t len, const enum temp_unit unit)
{
    char part[128];

    string_copy_span(part, sizeof(part), start, len);
    print_status_token(part, unit);
}

/**
 * Print human.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
static void print_status_human(const char* status, const enum temp_unit unit)
{
    const char* line = status;

    while (*line)
    {
        const char* line_end = strchr(line, '\n');
        const char* token = line;

        if (!line_end)
            line_end = line + strlen(line);

        while (token < line_end)
        {
            const char* token_end = token;

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

/**
 * Print client status.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
int client_print_status(const enum temp_unit unit, const bool json)
{
    char response[4096];

    if (json)
        return client_print_status_file();

    if (client_send_command_capture("status\n", true, response, sizeof(response)) == 0)
    {
        print_status_human(response, unit);

        return 0;
    }

    return client_print_status_file();
}

/**
 * Print coolboost.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
int client_print_coolboost_status(void)
{
    char response[256];

    if (client_send_command_capture("coolboost status\n", true, response, sizeof(response)) == 0)
    {
        fputs(response, stdout);

        return 0;
    }

    char* status = read_text_file(ANS_STATUS_PATH, 64 * 1024);

    if (!status)
    {
        perror(ANS_STATUS_PATH);

        return 1;
    }

    const char* enabled = strstr(status, "\"coolboost\": true");
    char* available = strstr(status, "\"coolboost_available\": true");

    if (!available)
    {
        printf("coolboost=unavailable\n");
    }
    else
    {
        printf("coolboost=%s\n", enabled ? "on" : "off");
    }

    free(status);

    return 0;
}
