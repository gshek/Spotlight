#include "compression_reader.h"

/*****************************************************************************/
/**************************** Constructor defines ****************************/
/*****************************************************************************/
#define NUMBER_AVAILABLE 3

/*****************************************************************************/
/*************** Imports of compression reader implementations ***************/
/*****************************************************************************/
#include "gzip/gzip_reader.h"
#include "xz/xz_reader.h"
#include "raw_read/reader.h"


/*****************************************************************************/
/*********************** Initialise available readers ************************/
/*****************************************************************************/
static CompressionReaderImpl available_readers[NUMBER_AVAILABLE] = {
    /* Highest precedence at the top */

    /* Gzip compression */
    {
        .is_supported = gzip_is_supported,
        .alloc = gzip_reader_alloc,
        .read = gzip_read,
        .free = gzip_reader_free
    },

    /* Blocked XZ compression */
    {
        .is_supported = xz_is_supported,
        .alloc = xz_reader_alloc,
        .read = xz_read,
        .free = xz_reader_free
    },


    /* Raw read (no compression) */
    {
        .is_supported = raw_read_is_supported,
        .alloc = raw_read_reader_alloc,
        .read = raw_read_read,
        .free = raw_read_reader_free
    }
};
