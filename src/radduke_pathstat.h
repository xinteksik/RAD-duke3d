#ifndef RADDUKE_PATHSTAT_H
#define RADDUKE_PATHSTAT_H
#include <stddef.h>
#include <sys/stat.h>
int radduke_stat_prefix(const char *prefix, size_t length, const char *name,
                        struct stat *info);
#endif
