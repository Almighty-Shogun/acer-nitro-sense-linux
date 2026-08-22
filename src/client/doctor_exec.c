#include "client/doctor_util.h"

#include "util/process.h"
#include "util/string.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void print_command_status(const char *label, const int status)
{
    if (status == -1)
        printf("%s=failed error=%s\n", label, strerror(errno));
    else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        printf("%s=exit-code-%d\n", label, WEXITSTATUS(status));
    else if (!WIFEXITED(status))
        printf("%s=not-exited\n", label);
}

static bool find_executable_in_path(const char *name, char *out,
                                    const size_t out_len)
{
    const char *path = getenv("PATH");
    char *copy;
    char *saveptr = NULL;
    bool found = false;

    if (!path || !path[0])
        return false;

    copy = strdup(path);
    if (!copy)
        return false;

    for (char *dir = strtok_r(copy, ":", &saveptr); dir;
         dir = strtok_r(NULL, ":", &saveptr)) {
        char candidate[PATH_MAX];
        const char *prefix = dir[0] ? dir : ".";

        if (snprintf(candidate, sizeof(candidate), "%s/%s", prefix, name) >=
            (int)sizeof(candidate))
            continue;

        if (access(candidate, X_OK) == 0) {
            string_copy(out, out_len, candidate);
            found = true;
            break;
        }
    }

    free(copy);
    return found;
}

void doctor_print_command_paths(void)
{
    static const char *const names[] = {
        "acer-nitro-sense",
        "ans",
        "acer-nitro-sensed",
    };

    printf("$ command -v acer-nitro-sense; command -v ans; command -v acer-nitro-sensed\n");

    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        char path[PATH_MAX];

        if (find_executable_in_path(names[i], path, sizeof(path)))
            printf("%s\n", path);
        else
            printf("%s=unavailable\n", names[i]);
    }
}

void doctor_run_command(const char *label, const char *command)
{
    const char *const argv[] = {"sh", "-c", command, NULL};
    char line[512];
    pid_t pid;
    int status;

    printf("$ %s\n", command);

    FILE *pipe = process_open_output("sh", argv, true, &pid);
    if (!pipe) {
        printf("%s=failed error=%s\n", label, strerror(errno));
        return;
    }

    while (fgets(line, sizeof(line), pipe))
        fputs(line, stdout);

    status = process_close_stdout(pipe, pid);
    print_command_status(label, status);
}
