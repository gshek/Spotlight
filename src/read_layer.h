#include <stdint.h>
#include <stdio.h>

#include <assert.h>

#include "compression/compression_reader.h"

/*****************************************************************************/
/***************************** Public functions ******************************/
/*****************************************************************************/

/** Read wrapper, handles state
 *
 *  @param file File to read
 *  @param buf Buffer to read data into
 *  @param offset Decompressed address to read from
 *  @param length Number of bytes to read, must be greater than size of buffer
 *
 *  @returns Number of bytes read or -1 if error
 */
extern int64_t read_wrapper(FILE *file, uint8_t *buf, off_t offset,
        size_t length);

/** Free read wrapper
 */
extern void free_read_wrapper();
