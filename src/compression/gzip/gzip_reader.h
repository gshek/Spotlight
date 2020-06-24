#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../libs/zlib-ng/zlib.h"

#define SPAN        1048576L    /* Desired distance between access points */
#define WINDOW_SIZE 32768U      /* Sliding window size */
#define CHUNK       16384       /* File input buffer size */

#define GZIP_WINDOW_BITS 47
#define RAW_INFLATE_BITS (-15)

#define INDEXER_ERROR   (-1)
#define TRUE            1
#define FALSE           0

#define GZIP_MAGIC_HEADER_SIZE  2
#define GZIP_MAGIC_HEADER       {0x1F, 0x8B}

/*****************************************************************************/
/********************************** Structs **********************************/
/*****************************************************************************/

/* Access point entry */
typedef struct GzipAccessPointEntry {

    /* Byte address in decompressed stream */
    off_t raw_byte_address;

    /* Byte address of first full byte in compressed file */
    off_t compressed_byte_address;

    /* Number of bits before compressed_byte_address for start of access */
    uint8_t bits;

    /* Context (window) */
    uint8_t context[WINDOW_SIZE];

    /* List fields */
    struct GzipAccessPointEntry *next;
    struct GzipAccessPointEntry *prev;
} GzipAccessPointEntry;

/* Access point list */
typedef struct GzipAccessPointList {

    /* Length of list */
    uint64_t length;

    /* First and last entries in list */
    struct GzipAccessPointEntry *first;
    struct GzipAccessPointEntry *last;
} GzipAccessPointList;

/* Indexer access fields, used for calling indexer functions */
typedef struct GzipReader {

    /* File pointer to gzip-compressed file */
    FILE *gzip_file;

    /* Pointer to access point list */
    struct GzipAccessPointList *list;
} GzipReader;

/*****************************************************************************/
/***************************** Public functions ******************************/
/*****************************************************************************/

/** Checks if gzip reader supports a particular file
 *
 *  @param gzip_file File pointer
 *
 *  @returns 1 for TRUE or 0 for FALSE
 */
extern uint8_t gzip_is_supported(FILE *gzip_file);

/** Allocates memory for gzip reader
 *
 *  @param gzip_file Gzip-compressed file to read from
 *
 *  @returns GzipReader structure, or NULL if error
 */
extern void *gzip_reader_alloc(FILE *gzip_file);

/** Reads from gzip-compresssed file
 *
 *  @param reader GzipReader that has been allocated
 *  @param buffer Buffer to read data into
 *  @param offset Decompressed address to read from
 *  @param length Number of bytes to read, must be greater than size of buffer
 *
 *  @returns Number of bytes read or INDEXER_ERROR if error
 */
extern int64_t gzip_read(void *reader, uint8_t *buffer,
        off_t offset, size_t length);

/** Frees memory for gzip reader
 *
 *  @param reader GzipReader structure
 */
extern void gzip_reader_free(void *reader);
