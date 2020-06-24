#include "reader.h"

/*****************************************************************************/
/***************************** Public functions ******************************/
/*****************************************************************************/

/* Returns true because no compression supports all files */
extern uint8_t raw_read_is_supported(FILE *file) {
    UNUSED(file);
    return TRUE;
}

/* Returns FILE pointer, no need to allocate any memory */
extern void *raw_read_reader_alloc(FILE *file) {
    return file;
}

/* Reads directly from file */
extern int64_t raw_read_read(void *file, uint8_t *buffer, off_t offset,
        size_t length) {
    int fd = fileno((FILE *) file);
    return pread(fd, buffer, length, offset);
}

/* Does nothing, nothing to free */
extern void raw_read_reader_free(void *file) {
    UNUSED(file);
}
