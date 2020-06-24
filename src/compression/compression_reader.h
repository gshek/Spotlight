#include <stdint.h>
#include <stdio.h>

#include <assert.h>

/*****************************************************************************/
/********************************** Structs **********************************/
/*****************************************************************************/

/* Structure for compression reader implementation */
typedef struct CompressionReaderImpl {

    /* Function that tests if file is supported by compression reader */
    uint8_t (*is_supported)(FILE *file);

    /* Function that allocates compression reader */
    void *(*alloc)(FILE *file);

    /* Function that reads data from file */
    int64_t (*read)(void *reader, uint8_t *buf, off_t offset, size_t length);

    /* Function that frees compression reader */
    void (*free)(void *reader);

} CompressionReaderImpl;

/* Structure for compression reader in use */
typedef struct CompressionReader {

    /* Allocated compression reader */
    void *reader;

    /* Structure for compression reader implementation */
    CompressionReaderImpl *reader_impl;

} CompressionReader;

/*****************************************************************************/
/***************************** Public functions ******************************/
/*****************************************************************************/

/** Allocates memory for compression reader
 *
 *  @param file file to read from
 *
 *  @returns CompressionReader structure, or NULL if error
 */
extern CompressionReader *compression_reader_alloc(FILE *file);

/** Reads from compressed file
 *
 *  @param reader CompressionReader that has been allocated
 *  @param buffer Buffer to read data into
 *  @param offset Decompressed address to read from
 *  @param length Number of bytes to read, must be greater than size of buffer
 *
 *  @returns Number of bytes read or -1 if error
 */
extern int64_t compression_read(CompressionReader *reader, uint8_t *buf,
        off_t offset, size_t length);

/** Frees memory for compression reader
 *
 *  @param reader CompressionReader structure
 */
extern void compression_reader_free(CompressionReader *reader);
