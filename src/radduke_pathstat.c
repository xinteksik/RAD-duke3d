#include "radduke_pathstat.h"
#include <errno.h>
#include <string.h>
int radduke_stat_prefix(const char *prefix, size_t length, const char *name,
                        struct stat *info)
{
    char path[1024];
    size_t size = strlen(name);
    int slash = length && prefix[length - 1] != '/' && prefix[length - 1] != '\\';
    if (length + slash + size >= sizeof(path)) { errno = ENAMETOOLONG; return -1; }
    memcpy(path, prefix, length);
    if (slash) path[length++] = '/';
    memcpy(path + length, name, size + 1);
    return stat(path, info);
}
