#include <stdint.h>
#include <stdio.h>

#include <assert.h>

#define NO_SUPPORTED_FUSE_IMPLEMENTATION    (-1)

/*****************************************************************************/
/********************************** Structs **********************************/
/*****************************************************************************/

/* Structure for fuse implementation */
typedef struct FuseImplementation {

    /* Function that tests if disk file is supported by fuse implementation */
    uint8_t (*is_supported)(int argc, char *argv[]);

    /* Fuse main function */
    int (*start)(int argc, char *argv[]);

} FuseImplementation;

/*****************************************************************************/
/***************************** Public functions ******************************/
/*****************************************************************************/

/** Starts fuse implementation
 *
 *  @param argc Fuse argc command line argument
 *  @param argv Fuse argc command line argument
 *
 *  @returns anything not 0 is error
 */
extern int start_fuse(int argc, char *argv[]);
