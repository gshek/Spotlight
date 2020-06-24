#include "constructors.h"

/* Returns first supported fuse implementation */
static FuseImplementation *get_supported_fuse_impl(int argc, char *argv[]) {
    for (uint32_t i = 0; i < NUMBER_AVAILABLE; i++) {
        if (available_fuse_implementations[i].is_supported(argc, argv)) {
            return &available_fuse_implementations[i];
        }
    }

    return NULL;
}

/* Starts fuse implementation */
extern int start_fuse(int argc, char *argv[]) {

    FuseImplementation *fuse_impl = get_supported_fuse_impl(argc, argv);
    if (fuse_impl == NULL) {
        return NO_SUPPORTED_FUSE_IMPLEMENTATION;
    }

    return fuse_impl->start(argc, argv);
}
