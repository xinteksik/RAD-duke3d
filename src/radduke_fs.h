#ifndef RADDUKE_FS_H
#define RADDUKE_FS_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
int radduke_fs_resolve(const char *path, char *out, size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
