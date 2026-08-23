#include "util/process.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Free a mutable argv copy built for exec.
 *
 * The public process API accepts const string literals, while POSIX exec uses
 * historical mutable pointer types. Keeping the copy local to this module
 * avoids spreading casts through callers.
 */
static void free_exec_argv(char** argv)
{
    if (!argv)
        return;

    for (size_t i = 0; argv[i]; i++)
        free(argv[i]);

    free(argv);
}

/**
 * Build a mutable argv copy for exec.
 *
 * exec does not mutate argument text, but its C API predates const-correct
 * prototypes. Duplicating the strings satisfies the API without discarding
 * qualifiers from the caller-owned argv.
 */
static char** copy_exec_argv(const char* const argv[])
{
    size_t argc = 0;

    while (argv[argc])
        argc++;

    char** copy = calloc(argc + 1, sizeof(*copy));

    if (!copy)
        return NULL;

    for (size_t i = 0; i < argc; i++)
    {
        copy[i] = strdup(argv[i]);

        if (!copy[i])
        {
            free_exec_argv(copy);

            return NULL;
        }
    }

    return copy;
}

/**
 * Start a child process and expose a stream for its output.
 *
 * The helper avoids shell interpolation and gives doctor/status code a simple
 * way to collect command output while still returning the child pid for an
 * explicit wait.
 */
FILE* process_open_output(const char* file, const char* const argv[], const bool stderr_to_stdout, pid_t* pid_out)
{
    int fds[2];

    if (!file || !argv || !pid_out)
    {
        errno = EINVAL;

        return NULL;
    }

    char** exec_argv = copy_exec_argv(argv);

    if (!exec_argv)
        return NULL;

    if (pipe(fds) < 0)
    {
        free_exec_argv(exec_argv);

        return NULL;
    }

    const pid_t pid = fork();

    if (pid < 0)
    {
        const int saved_errno = errno;

        close(fds[0]);
        close(fds[1]);
        free_exec_argv(exec_argv);

        errno = saved_errno;

        return NULL;
    }

    if (pid == 0)
    {
        close(fds[0]);

        if (dup2(fds[1], STDOUT_FILENO) < 0)
            _exit(126);

        if (stderr_to_stdout && dup2(fds[1], STDERR_FILENO) < 0)
            _exit(126);

        close(fds[1]);

        execvp(file, exec_argv);

        _exit(127);
    }

    close(fds[1]);
    free_exec_argv(exec_argv);

    FILE* stream = fdopen(fds[0], "r");

    if (!stream)
    {
        const int saved_errno = errno;

        close(fds[0]);

        while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {}

        errno = saved_errno;

        return NULL;
    }

    *pid_out = pid;

    return stream;
}

/**
 * Start a child process and expose stdout as a stream.
 *
 * This is the common path for probes where stderr should remain attached to
 * the caller rather than being folded into captured output.
 */
FILE* process_open_stdout(const char* file, const char* const argv[], pid_t* pid_out)
{
    return process_open_output(file, argv, false, pid_out);
}

/**
 * Close a captured process stream and wait for the child.
 *
 * Closing the stream first lets the reader finish consuming output before the
 * child status is collected.
 */
int process_close_stdout(FILE* stream, const pid_t pid)
{
    int status;
    pid_t waited;

    if (!stream)
    {
        errno = EINVAL;

        return -1;
    }

    const int close_result = fclose(stream);

    do
    {
        status = 0;
        waited = waitpid(pid, &status, 0);
    }
    while (waited < 0 && errno == EINTR);

    if (close_result < 0)
        return -1;

    if (waited < 0)
        return -1;

    return status;
}
