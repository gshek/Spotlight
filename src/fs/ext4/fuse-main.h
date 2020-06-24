#include <stdint.h>

#define FALSE   0
#define TRUE    1

/* Starts ext4fuse implementation */
int ext4fuse_main(int argc, char *argv[]);

/* Checks if ext4fuse implementation supports file */
uint8_t ext4fuse_is_supported(int argc, char *argv[]);
