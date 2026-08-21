#include "client/service.h"

#include "core/constants.h"
#include "util/process.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

static int run_output_command(const char *label, const char *file,
                              const char *const argv[])
{
    char line[512];
    pid_t pid;
    FILE *stream = process_open_output(file, argv, true, &pid);

    if (!stream) {
        fprintf(stderr, "%s: %s\n", label, strerror(errno));
        return 1;
    }

    while (fgets(line, sizeof(line), stream))
        fputs(line, stdout);

    const int status = process_close_stdout(stream, pid);
    if (status == -1) {
        fprintf(stderr, "%s: %s\n", label, strerror(errno));
        return 1;
    }
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        fprintf(stderr, "%s: terminated by signal %d\n", label, WTERMSIG(status));

    return 1;
}

int client_run_systemctl(const char *action)
{
    const char *const argv[] = {
        "systemctl",
        action,
        "acer-nitro-sense.service",
        NULL,
    };

    return run_output_command("systemctl", "systemctl", argv);
}

int client_validate_model(void)
{
    const char *const argv[] = {
        "acer-nitro-sensed",
        "--config",
        ANS_DEFAULT_CONFIG,
        "--validate-model",
        NULL,
    };

    return run_output_command("acer-nitro-sensed", "acer-nitro-sensed", argv);
}
