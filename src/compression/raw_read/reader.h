#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)

#define TRUE            1

/*****************************************************************************/
/***************************** Public functions ******************************/
/*****************************************************************************/

/** Returns true because no compression supports all files
 *
 *  @param file File pointer
 *
 *  @returns 1 for TRUE
 */
extern uint8_t raw_read_is_supported(FILE *file);

/** Returns FILE pointer
 *
 *  @param file File to read from
 *
 *  @returns file pointer
 */
extern void *raw_read_reader_alloc(FILE *file);

/** Reads directly from file
 *
 *  @param file File to read from
 *  @param buffer Buffer to read data into
 *  @param offset Address to read from
 *  @param length Number of bytes to read, must be greater than size of buffer
 *
 *  @returns Number of bytes read or -1 is returned
 */
extern int64_t raw_read_read(void *file, uint8_t *buffer, off_t offset,
        size_t length);

/** Does nothing
 *
 *  @param file File that was read
 */
extern void raw_read_reader_free(void *file);
