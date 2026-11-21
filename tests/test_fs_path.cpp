#include "radduke_fs.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
static void check(const char *input, const char *expected)
{
    char output[1024];
    assert(radduke_fs_resolve(input, output, sizeof(output)) == 0);
    assert(strcmp(output, expected) == 0);
}
int main()
{
    check("duke3d.grp", "SD:/RADDUKE/duke3d.grp");
    check("SD:RADDUKE", "SD:/RADDUKE");
    check("SD:/RADDUKE/./duke3d.grp", "SD:/RADDUKE/duke3d.grp");
    check("/RADDUKE/maps/../duke3d.grp", "SD:/RADDUKE/duke3d.grp");
    check("../../../../", "SD:/");
    check("a folder/file name.cfg", "SD:/RADDUKE/a folder/file name.cfg");
    check("/", "SD:/");
    char b[4];
    assert(radduke_fs_resolve("DUKE.GRP", b, sizeof(b)) == -1 && errno == ENAMETOOLONG);
    assert(radduke_fs_resolve("USB:DUKE.GRP", b, sizeof(b)) == -1 && errno == ENODEV);
    assert(radduke_fs_resolve("", b, sizeof(b)) == -1 && errno == ENOENT);
    assert(radduke_fs_resolve("a", NULL, 0) == -1 && errno == EINVAL);
    puts("FatFs virtual cwd/path normalization: OK");
}
