#include "client/doctor_util.h"

#include "util/string.h"
#include "util/process.h"

#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Print command status.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
static void print_command_status(const char* label, const int status)
{
    if (status == -1)
    {
        printf("%s=failed error=%s\n", label, strerror(errno));
    }
    else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        printf("%s=exit-code-%d\n", label, WEXITSTATUS(status));
    }
    else if (!WIFEXITED(status))
    {
        printf("%s=not-exited\n", label);
    }
}

/**
 * Find executable in path.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
static bool find_executable_in_path(const char* name, char* out, const size_t out_len)
{
    const char* path = getenv("PATH");

    char* saveptr = NULL;

    bool found = false;

    if (!path || !path[0])
        return false;

    char* copy = strdup(path);

    if (!copy)
        return false;

    for (const char* dir = strtok_r(copy, ":", &saveptr); dir; dir = strtok_r(NULL, ":", &saveptr))
    {
        char candidate[PATH_MAX];
        const char* prefix = dir[0] ? dir : ".";

        if (snprintf(candidate, sizeof(candidate), "%s/%s", prefix, name) >= (int)sizeof(candidate)) continue;

        if (access(candidate, X_OK) == 0)
        {
            string_copy(out, out_len, candidate);

            found = true;

            break;
        }
    }

    free(copy);

    return found;
}

/**
 * Print paths.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
void doctor_print_command_paths(void)
{
    static const char* const names[] = {
        "acer-nitro-sense",
        "ans",
        "acer-nitro-sensed",
    };

    printf("$ command -v acer-nitro-sense; command -v ans; command -v acer-nitro-sensed\n");

    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
    {
        char path[PATH_MAX];

        if (find_executable_in_path(names[i], path, sizeof(path)))
        {
            printf("%s\n", path);
        }
        else
        {
            printf("%s=unavailable\n", names[i]);
        }
    }
}

/**
 * Run one shell probe and print its output.
 *
 * The doctor report prints the exact command before execution so collected
 * diagnostics can be reproduced outside the tool.
 */
void doctor_run_command(const char* label, const char* command)
{
    const char* const argv[] = {"sh", "-c", command, NULL};

    char line[512];
    pid_t pid;

    printf("$ %s\n", command);

    FILE* pipe = process_open_output("sh", argv, true, &pid);

    if (!pipe)
    {
        printf("%s=failed error=%s\n", label, strerror(errno));

        return;
    }

    while (fgets(line, sizeof(line), pipe))
        fputs(line, stdout);

    const int status = process_close_stdout(pipe, pid);

    print_command_status(label, status);
}
