#include "util/file.h"

#include "util/fd.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int mkdir_p(const char *path)
{
    char tmp[512];
    const size_t len = strlen(path);

    if (len == 0 || len >= sizeof(tmp))
        return -1;

    memcpy(tmp, path, len + 1);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) < 0 && errno != EEXIST)
                return -1;
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) < 0 && errno != EEXIST)
        return -1;
    return 0;
}

char *read_text_file(const char *path, const size_t limit)
{
    FILE *f = fopen(path, "re");
    char *buf;
    size_t len;

    if (!f)
        return NULL;

    buf = calloc(1, limit + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    len = fread(buf, 1, limit, f);
    if (ferror(f)) {
        const int saved_errno = errno;

        fclose(f);
        free(buf);
        errno = saved_errno ? saved_errno : EIO;
        return NULL;
    }

    buf[len] = '\0';
    if (fclose(f) < 0) {
        free(buf);
        return NULL;
    }

    return buf;
}

int write_text_file_atomic(const char *path, const char *content)
{
    char tmp[512];
    const size_t len = strlen(content);

    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp))
        return -1;

    const int fd = open(tmp, O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, 0644);
    if (fd < 0)
        return -1;

    if (fd_write_all(fd, content, len) < 0) {
        const int saved_errno = errno;

        close(fd);
        unlink(tmp);
        errno = saved_errno;
        return -1;
    }

    if (fsync(fd) < 0) {
        const int saved_errno = errno;

        close(fd);
        unlink(tmp);
        errno = saved_errno;
        return -1;
    }

    if (close(fd) < 0) {
        const int saved_errno = errno;

        unlink(tmp);
        errno = saved_errno;
        return -1;
    }

    if (rename(tmp, path) < 0) {
        const int saved_errno = errno;

        unlink(tmp);
        errno = saved_errno;
        return -1;
    }

    return 0;
}
