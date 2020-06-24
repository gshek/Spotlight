#include "gzip_reader.h"

/*****************************************************************************/
/************************* Private struct functions **************************/
/*****************************************************************************/

/* Create an access point entry */
static GzipAccessPointEntry *create_access_point_entry(off_t raw_byte_address,
        off_t compressed_byte_address, uint8_t bits, uint64_t left,
        uint8_t *context) {
    GzipAccessPointEntry *new_entry;
    new_entry = (GzipAccessPointEntry *) malloc(sizeof(GzipAccessPointEntry));

    assert(new_entry != NULL);

    new_entry->raw_byte_address = raw_byte_address;
    new_entry->compressed_byte_address = compressed_byte_address;
    new_entry->bits = bits;
    memset(new_entry->context, 0, WINDOW_SIZE);
    if (left) {
        memcpy(new_entry->context, context + WINDOW_SIZE - left, left);
    }
    if (left < WINDOW_SIZE) {
        memcpy(new_entry->context + left, context, WINDOW_SIZE - left);
    }

    new_entry->next = NULL;
    new_entry->prev = NULL;

    return new_entry;
}

/* Deallocates an access point entry */
static void free_access_point_entry(GzipAccessPointEntry *entry) {
    assert(entry != NULL);

    if (entry->next) {
        entry->next->prev = entry->prev;
    }
    if (entry->prev) {
        entry->prev->next = entry->next;
    }
    free(entry);
}

/* Create an access point list */
static GzipAccessPointList *create_access_point_list() {
    GzipAccessPointList *new_list;
    new_list = (GzipAccessPointList *) malloc(sizeof(GzipAccessPointList));

    assert(new_list != NULL);

    new_list->length = 0;
    new_list->first = NULL;
    new_list->last = NULL;

    return new_list;
}

/* Append entry to access point list */
static void append_access_point_list(GzipAccessPointList *list,
        GzipAccessPointEntry *entry) {
    list->length++;

    entry->prev = list->last;
    entry->next = NULL;

    if (list->first) {
        list->last->next = entry;
    } else {
        list->first = entry;
    }
    list->last = entry;
}

/* Deallocates an access point list */
static void free_access_point_list(GzipAccessPointList *list) {
    assert(list != NULL);

    uint64_t freed_count = 0;
    GzipAccessPointEntry *current = list->first;
    GzipAccessPointEntry *next = current->next;

    while (next) {
        assert(next->prev == current);

        free_access_point_entry(current);
        freed_count++;
        current = next;
        next = next->next;
    }

    if (current) {
        assert(current == list->last);

        free_access_point_entry(current);
        freed_count++;
    }

    assert(list->length == freed_count);
    free(list);
}

/*****************************************************************************/
/**************************** Indexing functions *****************************/
/*****************************************************************************/

/* Processes the next chunk */
static int8_t index_next_chunk(GzipReader *reader, off_t max_byte_address,
        z_stream *stream, uint8_t *input_buf, uint8_t *context,
        off_t *raw_byte_counter, off_t *compressed_byte_counter) {

    int8_t return_value;

    stream->avail_in = fread(input_buf, 1, CHUNK, reader->gzip_file);
    if (ferror(reader->gzip_file)) {
        return Z_ERRNO;
    }
    if (stream->avail_in == 0) {
        return Z_DATA_ERROR;
    }
    stream->next_in = input_buf;
    stream->avail_out = 0;

    /* Process chunk, or until end of stream */
    do {
        /* Reset sliding window, if necessary */
        if (stream->avail_out == 0) {
            stream->avail_out = WINDOW_SIZE;
            stream->next_out = context;
        }

        /* Inflate until out of input, output, or at end of block */
        *compressed_byte_counter += stream->avail_in;
        *raw_byte_counter += stream->avail_out;

        /* Returns at end of block */
        return_value = inflate(stream, Z_BLOCK);

        *compressed_byte_counter -= stream->avail_in;
        *raw_byte_counter -= stream->avail_out;

        if (return_value == Z_MEM_ERROR
                || return_value == Z_DATA_ERROR
                || return_value == Z_NEED_DICT) {
            return return_value;
        } else if (return_value == Z_STREAM_END) {
            break;
        }

        if ((stream->data_type & 128) && !(stream->data_type & 64)
                && (*raw_byte_counter == 0 || *raw_byte_counter
                    - reader->list->last->raw_byte_address > SPAN)) {
            GzipAccessPointEntry *new_entry;
            new_entry = create_access_point_entry(*raw_byte_counter,
                    *compressed_byte_counter, stream->data_type & 7,
                    stream->avail_out, context);
            append_access_point_list(reader->list, new_entry);

            if (max_byte_address <
                    reader->list->last->raw_byte_address + SPAN) {
                break;
            }
        }
    } while (stream->avail_in != 0);

    return Z_OK;
}

/* Builds index up to max_byte_address */
static int8_t build_index(GzipReader *reader, off_t max_byte_address) {

    int8_t return_value;
    uint8_t context[WINDOW_SIZE];
    uint8_t input_buffer[CHUNK];
    off_t raw_byte_counter;
    off_t compressed_byte_counter;

    /* Initialise inflate */
    z_stream stream = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
        .avail_in = 0,
        .next_in = Z_NULL,
    };
    if (reader->list->length) {
        return_value = inflateInit2(&stream, RAW_INFLATE_BITS);
    } else {
        return_value = inflateInit2(&stream, GZIP_WINDOW_BITS);
    }
    stream.avail_out = 0;
    raw_byte_counter = 0;
    compressed_byte_counter = 0;

    if (reader->list->length) {
        GzipAccessPointEntry *entry = reader->list->last;
        raw_byte_counter = entry->raw_byte_address;
        compressed_byte_counter = entry->compressed_byte_address;
        if (entry->bits) {
            compressed_byte_counter--;
        }
        if (fseeko(reader->gzip_file, compressed_byte_counter, SEEK_SET) == -1) {
            inflateEnd(&stream);
            return ferror(reader->gzip_file) ? Z_ERRNO : Z_DATA_ERROR;
        }

        if (entry->bits) {
            compressed_byte_counter++;
            int next_char = getc(reader->gzip_file);
            if (next_char == -1) {
                inflateEnd(&stream);
                if (ferror(reader->gzip_file)) {
                    return Z_ERRNO;
                }
                return Z_DATA_ERROR;
            }
            inflatePrime(&stream, entry->bits, next_char >> (8 - entry->bits));
        }
        inflateSetDictionary(&stream, entry->context, WINDOW_SIZE);
    }

    while (return_value == Z_OK) {
        return_value = index_next_chunk(reader, max_byte_address,
                &stream, input_buffer, context, &raw_byte_counter,
                &compressed_byte_counter);
        if (max_byte_address < raw_byte_counter + SPAN) {
            break;
        }
    }

    inflateEnd(&stream);
    return return_value;
}

/* Checks if index needs to be built, and builds if required */
static int8_t check_and_build_index(GzipReader *reader,
        off_t max_byte_address) {
    if (reader->list->length) {
        assert(reader->list->last != NULL);

        off_t last_index = reader->list->last->raw_byte_address + SPAN;
        if (last_index > max_byte_address) {
            return 0;
        }
    }
    return build_index(reader, max_byte_address);
}

/*****************************************************************************/
/*************************** Data access functions ***************************/
/*****************************************************************************/

static int8_t read_next_chunk(GzipReader *reader,
        z_stream *stream, uint8_t *input_buffer) {

    if (stream->avail_in == 0) {
        stream->avail_in = fread(input_buffer, 1, CHUNK, reader->gzip_file);
        if (ferror(reader->gzip_file)) {
            return Z_ERRNO;
        }
        if (stream->avail_in == 0) {
            return Z_DATA_ERROR;
        }
        stream->next_in = input_buffer;
    }

    /* Normal inflate */
    return inflate(stream, Z_NO_FLUSH);
}

static int8_t _gzip_read(GzipReader *reader, uint8_t *buffer,
        off_t offset, uint64_t length) {

    if (length == 0) {
        return 0;
    }

    uint8_t skip;
    uint8_t input_buffer[CHUNK];
    uint8_t discard_window[WINDOW_SIZE];

    /* Find where in stream to start */
    // TODO: Improve searching because this is currently linear
    GzipAccessPointEntry *current = reader->list->first;
    assert(current != NULL);
    while (current->next && current->next->raw_byte_address < offset) {
        current = current->next;
    }

    int8_t return_value;

    /* Initialize file and inflate state to start there */
    /* Initialise inflate */
    z_stream stream = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
        .avail_in = 0,
        .next_in = Z_NULL,
    };
    return_value = inflateInit2(&stream, RAW_INFLATE_BITS);
    if (return_value != Z_OK) {
        inflateEnd(&stream);
        return return_value;
    }

    off_t comp_offset;
    comp_offset = current->compressed_byte_address - ((current->bits) ? 1 : 0);
    return_value = fseeko(reader->gzip_file, comp_offset, SEEK_SET);
    if (return_value == -1) {
        inflateEnd(&stream);
        return ferror(reader->gzip_file) ? Z_ERRNO : Z_DATA_ERROR;
    }

    if (current->bits) {
        int next_char = getc(reader->gzip_file);
        if (next_char == -1) {
            inflateEnd(&stream);
            return ferror(reader->gzip_file) ? Z_ERRNO : Z_DATA_ERROR;
        }
        inflatePrime(&stream, current->bits, next_char >> (8 - current->bits));
    }
    inflateSetDictionary(&stream, current->context, WINDOW_SIZE);

    /* Skip uncompressed bytes until offset reached, then satisfy request */
    offset -= current->raw_byte_address;
    stream.avail_in = 0;
    skip = 1;                               /* While skipping to offset */
    do {
        /* Define where to put uncompressed data, and how much */
        if (offset == 0 && skip) {
            /* At offset */
            stream.avail_out = length;
            stream.next_out = buffer;
            skip = 0;
        } else if (offset > WINDOW_SIZE) {
            /* Skip WINDOW_SIZE bytes */
            stream.avail_out = WINDOW_SIZE;
            stream.next_out = discard_window;
            offset -= WINDOW_SIZE;
        } else if (offset != 0) {
            /* Last skip */
            stream.avail_out = (unsigned) offset;
            stream.next_out = discard_window;
            offset = 0;
        }

        /* Uncompress until avail_out filled, or end of stream */
        do {
            return_value = read_next_chunk(reader, &stream,
                    input_buffer);
        } while (stream.avail_out != 0 && return_value == Z_OK);

        /* Do until offset reached and requested data read, or stream ends */
    } while (skip && return_value == Z_OK);

    /* Compute number of uncompressed bytes read after offset */
    inflateEnd(&stream);

    return return_value;
}

/*****************************************************************************/
/************************** Public struct functions **************************/
/*****************************************************************************/

/* Checks if gzip file */
extern uint8_t gzip_is_supported(FILE *gzip_file) {
    uint8_t magic_bytes[] = GZIP_MAGIC_HEADER;
    uint8_t file_header[GZIP_MAGIC_HEADER_SIZE];

    size_t ret = fread(file_header, 1, GZIP_MAGIC_HEADER_SIZE, gzip_file);

    assert(ret == GZIP_MAGIC_HEADER_SIZE);
    if (ret != GZIP_MAGIC_HEADER_SIZE) {
        return FALSE;
    }

    for (uint32_t i = 0; i < GZIP_MAGIC_HEADER_SIZE; i++) {
        if (file_header[i] != magic_bytes[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

/* Creates gzip reader struct */
extern void *gzip_reader_alloc(FILE *gzip_file) {
    GzipReader *reader = (GzipReader *) malloc(sizeof(GzipReader));

    assert(reader != NULL);

    if (reader != NULL) {
        reader->gzip_file = gzip_file;
        reader->list = create_access_point_list();
    }

    return (void *) reader;
}

/* Read bytes in gzip compressed file */
extern int64_t gzip_read(void *reader, uint8_t *buffer, off_t offset,
        size_t length) {

    GzipReader *gzip_reader = (GzipReader *) reader;
    int8_t return_value;

    return_value = check_and_build_index(gzip_reader, offset + length);
    return_value = (return_value == Z_STREAM_END) ? Z_OK : return_value;
    assert(return_value == Z_OK);

    return_value = _gzip_read(gzip_reader, buffer, offset, length);
    return_value = (return_value == Z_STREAM_END) ? Z_OK : return_value;
    assert(return_value == Z_OK);

    return (return_value == Z_OK) ? (signed) length : INDEXER_ERROR;
}

/* Free gzip reader struct */
extern void gzip_reader_free(void *reader) {
    GzipReader *gzip_reader = (GzipReader *) reader;
    free_access_point_list(gzip_reader->list);
    free(gzip_reader);
}
