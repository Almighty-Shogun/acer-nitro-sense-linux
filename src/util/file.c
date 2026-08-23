#include "util/file.h"

#include "util/fd.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Create one path segment, accepting an existing directory.
 *
 * `EEXIST` alone does not prove the path is usable, so an existing entry is
 * stat'd and anything that is not a directory fails with `ENOTDIR` rather than
 * reporting success the caller cannot rely on.
 */
static int mkdir_segment(const char* path)
{
    struct stat st;

    if (mkdir(path, 0755) == 0)
        return 0;

    if (errno != EEXIST)
        return -1;

    if (stat(path, &st) < 0)
        return -1;

    if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;

        return -1;
    }

    return 0;
}

/**
 * Create a directory path recursively.
 *
 * State and runtime paths may be missing on first startup. Each parent segment
 * is created with daemon-safe directory permissions, and an existing entry is
 * accepted only when it is already a directory.
 */
int mkdir_p(const char* path)
{
    char tmp[512];
    const size_t len = strlen(path);

    if (len == 0 || len >= sizeof(tmp))
        return -1;

    memcpy(tmp, path, len + 1);

    for (char* p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';

            if (mkdir_segment(tmp) < 0)
                return -1;

            *p = '/';
        }
    }

    if (mkdir_segment(tmp) < 0)
        return -1;

    return 0;
}

/**
 * Read a small text file into caller-owned memory.
 *
 * sysfs and procfs files are small but not always regular files, so callers
 * provide the maximum byte count they are willing to read.
 */
char* read_text_file(const char* path, const size_t limit)
{
    FILE* f = fopen(path, "re");

    if (!f)
        return NULL;

    char* buf = calloc(1, limit + 1);

    if (!buf)
    {
        fclose(f);

        return NULL;
    }

    const size_t len = fread(buf, 1, limit, f);

    if (ferror(f))
    {
        const int saved_errno = errno;

        fclose(f);
        free(buf);

        errno = saved_errno ? saved_errno : EIO;

        return NULL;
    }

    buf[len] = '\0';

    if (fclose(f) < 0)
    {
        free(buf);

        return NULL;
    }

    return buf;
}

/**
 * Atomically replace a text file.
 *
 * Persistent daemon state is written through a sibling temporary file and
 * renamed into place so readers never observe a partially written state file.
 */
int write_text_file_atomic(const char* path, const char* content)
{
    char tmp[512];
    const size_t len = strlen(content);

    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp))
        return -1;

    const int fd = open(tmp, O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, 0644);

    if (fd < 0)
        return -1;

    if (fd_write_all(fd, content, len) < 0)
    {
        const int saved_errno = errno;

        close(fd);
        unlink(tmp);

        errno = saved_errno;

        return -1;
    }

    if (fsync(fd) < 0)
    {
        const int saved_errno = errno;

        close(fd);
        unlink(tmp);

        errno = saved_errno;

        return -1;
    }

    if (close(fd) < 0)
    {
        const int saved_errno = errno;

        unlink(tmp);

        errno = saved_errno;

        return -1;
    }

    if (rename(tmp, path) < 0)
    {
        const int saved_errno = errno;

        unlink(tmp);

        errno = saved_errno;

        return -1;
    }

    return 0;
}
