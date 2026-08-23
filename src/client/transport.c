#include "client/transport.h"

#include "util/fd.h"
#include "util/file.h"
#include "core/constants.h"

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>

/**
 * Append capture output.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
static void append_capture_output(char* out, const size_t out_len, size_t* used, const char* text)
{
    if (!out || out_len == 0 || *used >= out_len - 1) return;

    const size_t available = out_len - *used - 1;
    size_t len = strlen(text);

    if (len > available)
        len = available;

    if (len == 0) return;

    memcpy(out + *used, text, len);

    *used += len;
    out[*used] = '\0';
}

/**
 * Print client status file.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
int client_print_status_file(void)
{
    char* status = read_text_file(ANS_STATUS_PATH, 64 * 1024);

    if (!status)
    {
        perror(ANS_STATUS_PATH);

        return 1;
    }

    fputs(status, stdout);
    free(status);

    return 0;
}

/**
 * Send a daemon command and keep its reply.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
int client_send_command_capture(const char* command, const bool quiet, char* out, const size_t out_len)
{
    struct sockaddr_un addr;
    char buf[4096];

    size_t used = 0;

    if (out_len > 0)
        out[0] = '\0';

    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if (fd < 0)
    {
        if (!quiet)
            perror("socket");

        return 1;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;

    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", ANS_SOCKET_PATH);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        if (!quiet)
            perror(ANS_SOCKET_PATH);

        close(fd);

        return 1;
    }

    if (fd_write_string(fd, command) < 0)
    {
        if (!quiet)
            perror("write");

        close(fd);

        return 1;
    }

    shutdown(fd, SHUT_WR);

    for (;;)
    {
        const ssize_t n = fd_read_retry(fd, buf, sizeof(buf) - 1);

        if (n < 0)
        {
            if (!quiet)
                perror("read");

            close(fd);

            return 1;
        }

        if (n == 0) break;

        buf[n] = '\0';

        if (out && out_len > 0)
        {
            append_capture_output(out, out_len, &used, buf);
        }
        else
        {
            fputs(buf, stdout);
        }
    }

    close(fd);

    return 0;
}

/**
 * Send a daemon command and print its reply.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
int client_send_command(const char* command, bool quiet)
{
    return client_send_command_capture(command, quiet, NULL, 0);
}

/**
 * Format and send a daemon command.
 *
 * The CLI uses this helper to keep terminal output and daemon transport
 * behavior consistent across subcommands.
 */
int client_send_commandf(const bool quiet, const char* format, ...)
{
    va_list args;

    char command[256];

    va_start(args, format);
    const int written = vsnprintf(command, sizeof(command), format, args);
    va_end(args);

    if (written < 0 || (size_t)written >= sizeof(command))
    {
        if (!quiet)
            fprintf(stderr, "command too long\n");

        return 2;
    }

    return client_send_command(command, quiet);
}
