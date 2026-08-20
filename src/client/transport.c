#include "client/transport.h"

#include "ans.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int client_print_status_file(void)
{
    char *status = read_text_file(ANS_STATUS_PATH, 64 * 1024);

    if (!status) {
        perror(ANS_STATUS_PATH);
        return 1;
    }

    fputs(status, stdout);
    free(status);
    return 0;
}

int client_send_command_capture(const char *command, bool quiet, char *out,
                                size_t out_len)
{
    struct sockaddr_un addr;
    char buf[4096];
    size_t used = 0;
    ssize_t n;

    if (out_len > 0)
        out[0] = '\0';

    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if (fd < 0) {
        if (!quiet)
            perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;

    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", ANS_SOCKET_PATH);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        if (!quiet)
            perror(ANS_SOCKET_PATH);

        close(fd);

        return 1;
    }

    if (write(fd, command, strlen(command)) != (ssize_t)strlen(command)) {
        if (!quiet)
            perror("write");

        close(fd);

        return 1;
    }

    shutdown(fd, SHUT_WR);

    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        if (out && out_len > 0) {
            size_t available = out_len - used - 1;
            size_t len = strlen(buf);

            if (len > available)
                len = available;

            if (len > 0) {
                memcpy(out + used, buf, len);
                used += len;
                out[used] = '\0';
            }
        } else {
            fputs(buf, stdout);
        }
    }

    close(fd);

    return 0;
}

int client_send_command(const char *command, bool quiet)
{
    return client_send_command_capture(command, quiet, NULL, 0);
}
