#include "constructors.h"

/* Returns first supported single compression reader implementation */
static CompressionReaderImpl *get_supported_comp_reader(FILE *file) {
    // TODO: Add empty file check, can be done by stat-ing the file
    for (uint32_t i = 0; i < NUMBER_AVAILABLE; i++) {
        if (available_readers[i].is_supported(file)) {
            return &available_readers[i];
        }
        rewind(file);
    }

    return NULL;
}

/* Allocates memory for compression reader */
extern CompressionReader *compression_reader_alloc(FILE *file) {
    CompressionReaderImpl *reader_impl = get_supported_comp_reader(file);
    rewind(file);
    if (reader_impl == NULL) {
        return NULL;
    }

    CompressionReader *reader;
    reader = (CompressionReader *) malloc(sizeof(CompressionReader));

    assert(reader != NULL);

    if (reader != NULL) {
        reader->reader_impl = reader_impl;
        reader->reader = reader_impl->alloc(file);

        assert(reader->reader != NULL);

        if (reader->reader == NULL) {
            free(reader);
            return NULL;
        }
    }
    return reader;
}


/* Reads from compressed file */
extern int64_t compression_read(CompressionReader *reader, uint8_t *buf,
        off_t offset, size_t length) {
    return reader->reader_impl->read(reader->reader, buf, offset, length);
}


/* Frees memory for compression reader */
extern void compression_reader_free(CompressionReader *reader) {
    reader->reader_impl->free(reader->reader);
    free(reader);
}
