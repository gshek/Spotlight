#include "read_layer.h"

#define FAILED_TO_ALLOC (-1)

static CompressionReader *compression_reader = NULL;

/* Read wrapper */
extern int64_t read_wrapper(FILE *file, uint8_t *buf, off_t offset,
        size_t length) {

    if (compression_reader == NULL) {
        compression_reader = compression_reader_alloc(file);

        assert(compression_reader != NULL);
        if (compression_reader == NULL) {
            return FAILED_TO_ALLOC;
        }
    }

    return compression_read(compression_reader, buf, offset, length);
}

/* Frees read wrapper */
extern void free_read_wrapper() {
    if (compression_reader != NULL) {
        compression_reader_free(compression_reader);
        compression_reader = NULL;
    }
}
