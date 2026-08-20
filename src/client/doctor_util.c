#include "client/doctor_util.h"

#include "ans.h"

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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
    char *line;

    if (!text) {
        printf("os_pretty_name=unavailable path=/etc/os-release error=%s\n",
               strerror(errno));
        return;
    }

    line = strtok(text, "\n");
    while (line) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char *value = line + 12;

            if (*value == '"') {
                value++;
                char *quote = strrchr(value, '"');
                if (quote)
                    *quote = '\0';
            }
            printf("os_pretty_name=%s\n", value);
            free(text);
            return;
        }
        line = strtok(NULL, "\n");
    }

    printf("os_pretty_name=unknown\n");
    free(text);
}

void doctor_run_command(const char *label, const char *command)
{
    char line[512];
    int status;

    printf("$ %s\n", command);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        printf("%s=failed error=%s\n", label, strerror(errno));
        return;
    }

    while (fgets(line, sizeof(line), pipe))
        fputs(line, stdout);

    status = pclose(pipe);
    if (status == -1)
        printf("%s=failed error=%s\n", label, strerror(errno));
    else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        printf("%s=exit-code-%d\n", label, WEXITSTATUS(status));
    else if (!WIFEXITED(status))
        printf("%s=not-exited\n", label);
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
