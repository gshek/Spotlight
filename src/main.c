#include <stdio.h>

#include "fs/fuse_implementation.h"

int main(int argc, char *argv[]) {
    int ret = start_fuse(argc, argv);

    if (ret != 0) {
        fprintf(stderr, "Usage: Disk Mountpoint\n");
    }
}
