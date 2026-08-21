#include "client/doctor_util.h"

#include "core/constants.h"
#include "util/file.h"
#include "util/process.h"
#include "util/string.h"

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static bool os_release_pretty_name(const char *text, char *out,
                                   const size_t out_len)
{
    const char *line = text;

    while (*line) {
        const char *line_end = strchr(line, '\n');
        const char *value;
        size_t len;

        if (!line_end)
            line_end = line + strlen(line);

        if (strncmp(line, "PRETTY_NAME=", 12) != 0) {
            line = *line_end == '\n' ? line_end + 1 : line_end;
            continue;
        }

        value = line + 12;
        len = (size_t)(line_end - value);
        if (len >= 2 && value[0] == '"' && value[len - 1] == '"') {
            value++;
            len -= 2;
        }

        string_copy_span(out, out_len, value, len);
        return true;
    }

    return false;
}

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

void doctor_print_os_pretty_name(void)
{
    char *text = read_text_file("/etc/os-release", 64 * 1024);
    char pretty_name[256];

    if (!text) {
        printf("os_pretty_name=unavailable path=/etc/os-release error=%s\n",
               strerror(errno));
        return;
    }

    if (os_release_pretty_name(text, pretty_name, sizeof(pretty_name)))
        printf("os_pretty_name=%s\n", pretty_name);
    else
        printf("os_pretty_name=unknown\n");

    free(text);
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

void doctor_print_model_config(void)
{
    const char *path = ANS_DEFAULT_CONFIG;
    struct stat st;
    char target[PATH_MAX];
    char resolved[PATH_MAX];

    printf("$ inspect %s\n", path);

    if (lstat(path, &st) < 0) {
        printf("config=unavailable path=%s error=%s\n", path, strerror(errno));
        return;
    }

    printf("config=%s mode=%04o uid=%ld gid=%ld", path,
           (unsigned int)(st.st_mode & 07777), (long)st.st_uid,
           (long)st.st_gid);

    if (S_ISLNK(st.st_mode)) {
        const ssize_t len = readlink(path, target, sizeof(target) - 1);

        if (len >= 0) {
            target[len] = '\0';
            printf(" target=%s", target);
        }
    }
    putchar('\n');

    if (realpath(path, resolved))
        printf("%s\n", resolved);
    else
        printf("config_realpath=unavailable path=%s error=%s\n", path,
               strerror(errno));
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

void doctor_print_socket_permissions(void)
{
    struct stat st;
    struct group *group;

    if (stat(ANS_SOCKET_PATH, &st) < 0) {
        printf("socket=unavailable path=%s error=%s\n", ANS_SOCKET_PATH, strerror(errno));
        return;
    }

    group = getgrgid(st.st_gid);
    printf("socket=%s mode=%04o uid=%ld gid=%ld group=%s\n",
           ANS_SOCKET_PATH, (unsigned int)(st.st_mode & 07777),
           (long)st.st_uid, (long)st.st_gid, group ? group->gr_name : "unknown");
}

void doctor_print_user_groups(void)
{
    struct passwd *passwd = getpwuid(getuid());
    int ngroups = 0;
    gid_t *groups = NULL;

    printf("uid=%ld user=%s gid=%ld\n", (long)getuid(),
           passwd ? passwd->pw_name : "unknown", (long)getgid());

    getgroups(0, NULL);
    ngroups = getgroups(0, NULL);
    if (ngroups <= 0)
        return;

    groups = calloc((size_t)ngroups, sizeof(*groups));
    if (!groups)
        return;

    if (getgroups(ngroups, groups) == ngroups) {
        printf("groups=");
        for (int i = 0; i < ngroups; i++) {
            struct group *group = getgrgid(groups[i]);
            printf("%s%ld%s%s", i == 0 ? "" : ",", (long)groups[i],
                   group ? ":" : "", group ? group->gr_name : "");
        }
        putchar('\n');
    }

    free(groups);
}
