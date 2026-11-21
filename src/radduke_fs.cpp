/* FatFs-backed POSIX subset required by JFBuild; no FF_FS_RPATH change. */
#include "radduke_fs.h"
#ifndef RADDUKE_FS_PATH_TEST_ONLY
#include "wrap_fatfs.h"
#include <circle/memory.h>
#include "radduke_platform.h"
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef RADDUKE_FS_PATH_TEST_ONLY
#undef errno
extern int errno;
#endif
static char radduke_cwd[1024] = "/RADDUKE";
extern "C" int radduke_fs_resolve(const char *path, char *out, size_t capacity)
{
    char joined[2048];
    if (!out || !capacity) { errno = EINVAL; return -1; }
    if (!path || !*path) { errno = ENOENT; return -1; }
    int drive = !strncmp(path, "SD:", 3) || !strncmp(path, "sd:", 3);
    if (drive) path += 3;
    if (strchr(path, ':')) { errno = ENODEV; return -1; }
    size_t prefix = drive || path[0] == '/' ? 0 : strlen(radduke_cwd);
    if (prefix + strlen(path) + 2 >= sizeof(joined)) { errno = ENAMETOOLONG; return -1; }
    memcpy(joined, radduke_cwd, prefix);
    joined[prefix] = '/'; strcpy(joined + prefix + 1, path);
    char normalized[1024] = "SD:";
    size_t length = 3;
    const char *p = joined;
    while (*p) {
        while (*p == '/' || *p == '\\') ++p;
        const char *begin = p;
        while (*p && *p != '/' && *p != '\\') ++p;
        size_t n = (size_t)(p - begin);
        if (!n || (n == 1 && begin[0] == '.')) continue;
        if (n == 2 && begin[0] == '.' && begin[1] == '.') {
            while (length > 3 && normalized[length - 1] != '/') --length;
            if (length > 3) --length;
            continue;
        }
        if (length + n + 2 >= sizeof(normalized)) { errno = ENAMETOOLONG; return -1; }
        normalized[length++] = '/';
        memcpy(normalized + length, begin, n); length += n;
    }
    if (length == 3) normalized[length++] = '/';
    normalized[length] = 0;
    if (length + 1 > capacity) { errno = ENAMETOOLONG; return -1; }
    memcpy(out, normalized, length + 1);
    return 0;
}
#ifndef RADDUKE_FS_PATH_TEST_ONLY
extern "C" char *getcwd(char *out, size_t capacity)
{
    size_t n = strlen(radduke_cwd) + 1;
    if ((out && capacity < n) || (!out && capacity && capacity < n)) { errno = ERANGE; return NULL; }
    if (!out) { out = (char *)malloc(capacity ? capacity : n); if (!out) { errno = ENOMEM; return NULL; } }
    memcpy(out, radduke_cwd, n); return out;
}
extern "C" int _stat(const char *name, struct stat *st)
{
    char path[1024]; FILINFO info;
    if (radduke_fs_resolve(name, path, sizeof(path))) return -1;
    FRESULT r = f_stat(path, &info);
    if (r != FR_OK) { errno = r == FR_NO_FILE || r == FR_NO_PATH ? ENOENT : EIO; return -1; }
    memset(st, 0, sizeof(*st));
    st->st_mode = (info.fattrib & AM_DIR ? S_IFDIR : S_IFREG) | 0400 | (info.fattrib & AM_RDO ? 0 : 0200);
    st->st_size = info.fsize;
    st->st_mtime = ((unsigned)info.fdate << 16) | info.ftime;
    return 0;
}
extern "C" int _fstat(int fd, struct stat *st)
{
    memset(st, 0, sizeof(*st));
    if (fd >= 0 && fd < 3) { st->st_mode = S_IFCHR | 0600; return 0; }
    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos < 0) return -1;
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) return -1;
    if (lseek(fd, pos, SEEK_SET) < 0) return -1;
    st->st_mode = S_IFREG | 0600; st->st_size = size; return 0;
}
extern "C" int chdir(const char *name)
{
    char absolute[1024]; struct stat info;
    if (radduke_fs_resolve(name, absolute, sizeof(absolute)) || _stat(absolute, &info)) return -1;
    if (!S_ISDIR(info.st_mode)) { errno = ENOTDIR; return -1; }
    strcpy(radduke_cwd, absolute + 3); return 0;
}
extern "C" int mkdir(const char *name, mode_t mode)
{
    (void)mode; char path[1024];
    if (radduke_fs_resolve(name, path, sizeof(path))) return -1;
    FRESULT r = f_mkdir(path);
    if (r == FR_OK) return 0;
    errno = r == FR_EXIST ? EEXIST : EIO; return -1;
}
extern "C" int access(const char *name, int mode)
{
    struct stat info;
    if (mode & ~(R_OK | W_OK | X_OK)) { errno = EINVAL; return -1; }
    if (_stat(name, &info)) return -1;
    if ((mode & W_OK) && !(info.st_mode & 0200)) { errno = EACCES; return -1; }
    if ((mode & X_OK) && !S_ISDIR(info.st_mode)) { errno = EACCES; return -1; }
    return 0;
}
extern "C" long sysconf(int name)
{
    if (name == _SC_PAGESIZE) return PAGE_SIZE;
    if (name == _SC_PHYS_PAGES && radduke_memory_bytes) return (long)(radduke_memory_bytes / PAGE_SIZE);
    errno = EINVAL; return -1;
}
#endif
