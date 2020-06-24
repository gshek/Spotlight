#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lzma.h>

#define MAX_SUPPORTED_BLOCK_SIZE_MB     64
#define MAX_NUM_BLOCKS_CACHE            32
#define IO_BUFFER_SIZE                  16384

#define READER_ERROR   (-1)
#define TRUE            1
#define FALSE           0

#define XZ_MAGIC_HEADER_SIZE  6
#define XZ_MAGIC_HEADER       {0xFD, '7', 'z', 'X', 'Z', 0x00}

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/*****************************************************************************/
/********************************** Structs **********************************/
/*****************************************************************************/

/* XzBlockCacheEntry */
typedef struct XzBlockCacheEntry {
    /* Uncompressed address */
    off_t offset;

    /* Block size */
    size_t size;

    /* Entire block */
    uint8_t *data;

    /* Next and previous pointers */
    struct XzBlockCacheEntry *next;
    struct XzBlockCacheEntry *prev;
} XzBlockCacheEntry;

/* XZ block cache */
typedef struct XzBlockCache {

    /* Number cached */
    uint64_t num_blocks;

    /* Cache entry list */
    XzBlockCacheEntry *first;
    XzBlockCacheEntry *last;

} XzBlockCache;

/* XZ compression reader */
typedef struct XzReader {

    /* File pointer to xz-compressed file */
    FILE *xz_file;

    /* Contains index to blocks in xz file */
    lzma_index *index;

    /* Total amount of stream padding */
    uint64_t stream_padding;

    /* Block cache */
    XzBlockCache cache;

} XzReader;

/*****************************************************************************/
/***************************** Public functions ******************************/
/*****************************************************************************/

/** Checks if xz reader supports a particular file
 *
 *  @param xz_file File pointer
 *
 *  @returns 1 for TRUE or 0 for FALSE
 */
extern uint8_t xz_is_supported(FILE *xz_file);

/** Allocates memory for xz reader
 *
 *  @param xz_file xz-compressed file to read from
 *
 *  @returns XzReader structure, or NULL if error
 */
extern void *xz_reader_alloc(FILE *xz_file);

/** Reads from xz-compresssed file
 *
 *  @param reader XzReader that has been allocated
 *  @param buffer Buffer to read data into
 *  @param offset Decompressed address to read from
 *  @param length Number of bytes to read, must be greater than size of buffer
 *
 *  @returns Number of bytes read or READER_ERROR if error
 */
extern int64_t xz_read(void *reader, uint8_t *buffer,
        off_t offset, size_t length);

/** Frees memory for xz reader
 *
 *  @param reader XzReader structure
 */
extern void xz_reader_free(void *reader);
