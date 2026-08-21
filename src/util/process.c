#include "util/process.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

FILE *process_open_stdout(const char *file, char *const argv[], pid_t *pid_out)
{
    int fds[2];
    pid_t pid;

    if (!file || !argv || !pid_out) {
        errno = EINVAL;
        return NULL;
    }

    if (pipe(fds) < 0)
        return NULL;

    pid = fork();
    if (pid < 0) {
        const int saved_errno = errno;

        close(fds[0]);
        close(fds[1]);
        errno = saved_errno;
        return NULL;
    }

    if (pid == 0) {
        close(fds[0]);
        if (dup2(fds[1], STDOUT_FILENO) < 0)
            _exit(126);
        close(fds[1]);
        execvp(file, argv);
        _exit(127);
    }

    close(fds[1]);
    FILE *stream = fdopen(fds[0], "r");
    if (!stream) {
        const int saved_errno = errno;

        close(fds[0]);
        while (waitpid(pid, NULL, 0) < 0 && errno == EINTR)
            ;
        errno = saved_errno;
        return NULL;
    }

    *pid_out = pid;
    return stream;
}

int process_close_stdout(FILE *stream, const pid_t pid)
{
    int status;
    int close_result;
    pid_t waited;

    if (!stream) {
        errno = EINVAL;
        return -1;
    }

    close_result = fclose(stream);
    do {
        status = 0;
        waited = waitpid(pid, &status, 0);
    } while (waited < 0 && errno == EINTR);

    if (close_result < 0)
        return -1;
    if (waited < 0)
        return -1;

    return status;
}
