#include "xz_reader.h"

/*****************************************************************************/
/************************* Private struct functions **************************/
/*****************************************************************************/

/* Moves a cache entry to start of linked list */
static void move_to_cache_head(XzBlockCache *cache, XzBlockCacheEntry *entry) {
    if (cache->first != entry) {
        if (entry->next) {
            entry->next->prev = entry->prev;
        } else {
            assert(entry == cache->last);
            cache->last = entry->prev;
        }
        entry->prev->next = entry->next;

        entry->prev = NULL;
        entry->next = cache->first;
        entry->next->prev = entry;
        cache->first = entry;
    }
}

/* Get block from cache, or do not get block */
static uint8_t get_cache_block(XzBlockCache *cache, uint8_t **buffer,
        off_t *start, size_t *size, off_t offset) {

    for (XzBlockCacheEntry *entry = cache->first; entry; entry = entry->next) {
        off_t entry_end = entry->offset + (long) entry->size;
        if (entry->offset <= offset && offset < entry_end) {
            buffer[0] = entry->data;
            start[0] = entry->offset;
            size[0] = entry->size;
            move_to_cache_head(cache, entry);
            return TRUE;
        }
    }

    return FALSE;
}

/* Add new cache block entry, using LRU cache replacement policy */
static uint8_t add_new_block(XzBlockCache *cache, uint8_t *block, off_t offset,
        size_t size) {

    if (MAX_NUM_BLOCKS_CACHE == 0) {
        return FALSE;
    }

    if (cache->num_blocks < MAX_NUM_BLOCKS_CACHE) {
        XzBlockCacheEntry *new_entry;
        new_entry = (XzBlockCacheEntry *) malloc(sizeof(XzBlockCacheEntry));
        assert(new_entry != NULL);

        if (new_entry == NULL) {
            return FALSE;
        }

        new_entry->data = block;
        new_entry->offset = offset;
        new_entry->size = size;

        new_entry->prev = NULL;
        new_entry->next = cache->first;
        if (cache->first) {
            cache->first->prev = new_entry;
        }
        cache->first = new_entry;
        if (cache->last == NULL) {
            assert(cache->num_blocks == 0);
            cache->last = new_entry;
        }
        cache->num_blocks++;

    } else {
        move_to_cache_head(cache, cache->last);
        free(cache->first->data);
        cache->first->data = block;
        cache->first->offset = offset;
        cache->first->size = size;
    }
    return TRUE;
}

/* Free cache entries */
static void free_cache_entries(XzBlockCache *cache) {
    uint64_t count = 0;
    XzBlockCacheEntry *current = cache->first;
    XzBlockCacheEntry *temp_prev;

    if (current) {
        assert(current->prev == NULL);
        assert(cache->last->next == NULL);
    }

    while (current) {
        temp_prev = current;
        current = current->next;

        if (current) {
            assert(temp_prev == current->prev);
        }

        free(temp_prev->data);
        free(temp_prev);

        count++;
    }

    assert(count == cache->num_blocks);
    cache->num_blocks = 0;
    cache->first = NULL;
    cache->last = NULL;
}

/*****************************************************************************/
/*************************** Data access functions ***************************/
/*****************************************************************************/

/* Based on code from: https://github.com/xz-mirror/xz */
static lzma_ret parse_block_indexes(XzReader *reader) {

    assert(reader->index == NULL);

    lzma_ret return_value = LZMA_OK;
    int32_t file_fd = fileno(reader->xz_file);

    uint8_t buffer[IO_BUFFER_SIZE];
    lzma_stream stream = LZMA_STREAM_INIT;
    lzma_index *current_index = NULL;
    lzma_stream_flags header_flags;
    lzma_stream_flags footer_flags;

    /* Seek to end of file */
    fseek(reader->xz_file, 0, SEEK_END);

    /* Current position in file */
    off_t position = ftell(reader->xz_file);

    /* Each loop iteration decodes one index */
    do {
        /* Checks there is enough data to contain stream header and footer */
        if (position < 2 * LZMA_STREAM_HEADER_SIZE) {
            return_value = LZMA_DATA_ERROR;
            goto error;
        }

        position -= LZMA_STREAM_HEADER_SIZE;
        lzma_vli stream_padding = 0;

        /* Locate stream footer */
        while (TRUE) {
            if (position < LZMA_STREAM_HEADER_SIZE) {
                return_value = LZMA_DATA_ERROR;
                goto error;
            }

            if (pread(file_fd, &buffer, LZMA_STREAM_HEADER_SIZE, position)
                    != LZMA_STREAM_HEADER_SIZE) {
                return_value = LZMA_PROG_ERROR;
                goto error;
            }

            /* Stream padding is always multiple of 4 bytes */
            int32_t i = 2;
            if (buffer[i] != 0) {
                break;
            }

            /* To avoid calling pread for every 4 bytes of stream padding */
            do {
                stream_padding += 4;
                position -= 4;
                i--;
            } while (i >= 0 && buffer[i] == 0);
        }

        /* Decode stream footer */
        return_value = lzma_stream_footer_decode(&footer_flags, buffer);
        if (return_value != LZMA_OK) {
            goto error;
        }

        /* Check stream footer is supported */
        if (footer_flags.version != 0) {
            return_value = LZMA_OPTIONS_ERROR;
            goto error;
        }

        /* Check size of index field looks sane */
        lzma_vli index_size = footer_flags.backward_size;
        if ((lzma_vli) position < index_size + LZMA_STREAM_HEADER_SIZE) {
            return_value = LZMA_DATA_ERROR;
            goto error;
        }

        /* Set position to start of index */
        position -= index_size;

        /* Decode index */
        return_value = lzma_index_decoder(&stream, &current_index, INT64_MAX);
        if (return_value != LZMA_OK) {
            goto error;
        }

        do {
            long pread_size = MIN(IO_BUFFER_SIZE, (long) index_size);
            stream.avail_in = pread_size;
            if (pread(file_fd, &buffer, stream.avail_in, position)
                    != pread_size) {
                return_value = LZMA_PROG_ERROR;
                goto error;
            }

            position += stream.avail_in;
            index_size -= stream.avail_in;

            stream.next_in = buffer;
            return_value = lzma_code(&stream, LZMA_RUN);
        } while (return_value == LZMA_OK);

        /* Check index decoder consumed correct input amount */
        if (return_value == LZMA_STREAM_END) {
            if (index_size != 0 || stream.avail_in != 0) {
                return_value = LZMA_DATA_ERROR;
            }
        }

        if (return_value != LZMA_STREAM_END) {
            goto error;
        }

        /* Decode stream header and check stream flags */
        position -= footer_flags.backward_size + LZMA_STREAM_HEADER_SIZE;
        if ((lzma_vli) position < lzma_index_total_size(current_index)) {
            return_value = LZMA_DATA_ERROR;
            goto error;
        }

        position -= lzma_index_total_size(current_index);
        if (pread(file_fd, &buffer, LZMA_STREAM_HEADER_SIZE, position)
                != LZMA_STREAM_HEADER_SIZE) {
            return_value = LZMA_PROG_ERROR;
            goto error;
        }

        return_value = lzma_stream_header_decode(&header_flags, buffer);
        if (return_value != LZMA_OK) {
            goto error;
        }

        return_value = lzma_stream_flags_compare(&header_flags, &footer_flags);
        if (return_value != LZMA_OK) {
            goto error;
        }

        /* Store decoded stream flags into current_index */
        return_value = lzma_index_stream_flags(current_index, &footer_flags);

        /* Store size of stream padding field */
        return_value = lzma_index_stream_padding(current_index,
                stream_padding);

        if (reader->index != NULL) {
            return_value = lzma_index_cat(current_index, reader->index, NULL);
            if (return_value != LZMA_OK) {
                goto error;
            }
        }

        reader->index = current_index;
        current_index = NULL;

        reader->stream_padding += stream_padding;

    } while (position > 0);

    lzma_end(&stream);

    return return_value;

error:
    lzma_end(&stream);
    lzma_index_end(reader->index, NULL);
    lzma_index_end(current_index, NULL);
    return return_value;
}

/* Base on code from: https://github.com/libguestfs/nbdkit */
static lzma_ret read_block(XzReader *reader, uint8_t **data, off_t offset,
        off_t *block_start, size_t *block_size) {

    off_t compressed_offset;
    size_t size;
    lzma_index_iter iter;
    uint8_t header[LZMA_BLOCK_HEADER_SIZE_MAX];
    lzma_block block;
    lzma_filter filters[LZMA_FILTERS_MAX + 1];
    lzma_ret return_value;
    lzma_stream stream = LZMA_STREAM_INIT;
    uint8_t buffer[IO_BUFFER_SIZE];
    int32_t file_fd = fileno(reader->xz_file);

    /* Locate block containing uncompressed offset */
    lzma_index_iter_init(&iter, reader->index);
    if (lzma_index_iter_locate(&iter, offset)) {
        return LZMA_PROG_ERROR;
    }

    fseek(reader->xz_file, 0, SEEK_END);
    size = ftell(reader->xz_file);

    *block_start = iter.block.uncompressed_file_offset;
    *block_size = iter.block.uncompressed_size;

    /* Find size of block header by reading single byte */
    compressed_offset = iter.block.compressed_file_offset;
    if (pread(file_fd, header, 1, compressed_offset) != 1) {
        return LZMA_PROG_ERROR;
    }
    compressed_offset++;
    if (header[0] == '\0') {
        return LZMA_DATA_ERROR;
    }

    block.version = 0;
    block.check = iter.stream.flags->check;
    block.filters = filters;
    block.header_size = lzma_block_header_size_decode(header[0]);
    block.uncompressed_size = iter.block.uncompressed_size;

    /* Read and decode block header */
    if (pread(file_fd, &header[1], block.header_size - 1, compressed_offset)
            != block.header_size - 1) {
        return LZMA_DATA_ERROR;
    }
    compressed_offset += block.header_size - 1;

    return_value = lzma_block_header_decode(&block, NULL, header);
    if (return_value != LZMA_OK) {
        return return_value;
    }

    /* Check block header matches index */
    return_value = lzma_block_compressed_size(&block,
            iter.block.unpadded_size);
    if (return_value != LZMA_OK) {
        goto error1;
    }

    /* Read block data */
    return_value = lzma_block_decoder(&stream, &block);
    if (return_value != LZMA_OK) {
        goto error1;
    }

    *data = (uint8_t *) malloc(sizeof(uint8_t) * block_size[0]);
    if (data[0] == NULL) {
        goto error2;
    }

    stream.next_in = NULL;
    stream.avail_in = 0;
    stream.next_out = (uint8_t *) data[0];
    stream.avail_out = block.uncompressed_size;
    do {
        if (stream.avail_in == 0) {
            stream.avail_in = IO_BUFFER_SIZE;
            if (compressed_offset + stream.avail_in > size) {
                stream.avail_in = size - compressed_offset;
            }
            if (stream.avail_in > 0) {
                stream.next_in = buffer;
                if (pread(file_fd, buffer, stream.avail_in, compressed_offset)
                        != (unsigned) stream.avail_in) {
                    goto error2;
                }
                compressed_offset += stream.avail_in;
            }
        }

        return_value = lzma_code(&stream, LZMA_RUN);
    } while (return_value == LZMA_OK);

    if (return_value != LZMA_OK && return_value != LZMA_STREAM_END) {
        goto error2;
    }

    lzma_end(&stream);

    for (size_t i = 0; filters[i].id != LZMA_VLI_UNKNOWN; i++) {
        free(filters[i].options);
    }

    return LZMA_OK;

error2:
    lzma_end(&stream);

error1:
    for (size_t i = 0; filters[i].id != LZMA_VLI_UNKNOWN; i++) {
        free(filters[i].options);
    }

    free(*data);

    return return_value;
}

/*****************************************************************************/
/************************** Public struct functions **************************/
/*****************************************************************************/

/* Checks if xz file */
extern uint8_t xz_is_supported(FILE *xz_file) {
    uint8_t magic_bytes[] = XZ_MAGIC_HEADER;
    uint8_t file_header[XZ_MAGIC_HEADER_SIZE];

    size_t ret = fread(file_header, 1, XZ_MAGIC_HEADER_SIZE, xz_file);

    assert(ret == XZ_MAGIC_HEADER_SIZE);
    if (ret != XZ_MAGIC_HEADER_SIZE) {
        return FALSE;
    }

    for (uint32_t i = 0; i < XZ_MAGIC_HEADER_SIZE; i++) {
        if (file_header[i] != magic_bytes[i]) {
            return FALSE;
        }
    }

    XzReader reader = {
        .xz_file = xz_file,
        .index = NULL,
        .stream_padding = 0
    };
    if (parse_block_indexes(&reader) != LZMA_OK) {
        return FALSE;
    } else {
        lzma_index_iter iter;
        uint64_t max_block_size = 0;

        lzma_index_iter_init(&iter, reader.index);
        while (!lzma_index_iter_next(&iter, LZMA_INDEX_ITER_NONEMPTY_BLOCK)) {
            if (iter.block.uncompressed_size > max_block_size) {
                max_block_size = iter.block.uncompressed_size;
            }
        }

        uint64_t max_supported = MAX_SUPPORTED_BLOCK_SIZE_MB * 1024 * 1024;

        if (max_block_size > max_supported) {
            lzma_index_end(reader.index, NULL);
            return FALSE;
        }
    }

    return TRUE;
}

/* Creates xz reader struct */
extern void *xz_reader_alloc(FILE *xz_file) {
    XzReader *reader = (XzReader *) malloc(sizeof(XzReader));
    assert(reader != NULL);

    if (reader != NULL) {
        reader->xz_file = xz_file;
        reader->index = NULL;

        reader->cache.num_blocks = 0;
        reader->cache.first = NULL;
        reader->cache.last = NULL;

        if (parse_block_indexes(reader) != LZMA_OK) {
            xz_reader_free((void *) reader);
            return NULL;
        }
    }

    return (void *) reader;
}

/* Read bytes in xz compressed file */
extern int64_t xz_read(void *reader, uint8_t *buffer, off_t offset,
        size_t length) {

    XzReader *xz_reader = (XzReader *) reader;
    XzBlockCache *cache = &xz_reader->cache;

    uint8_t *block;
    uint8_t cache_hit;
    off_t start;
    size_t size;

    cache_hit = get_cache_block(cache, &block, &start, &size, offset);

    if (!cache_hit) {
        lzma_ret ret = read_block(xz_reader, &block, offset, &start, &size);
        if (ret != LZMA_OK) {
            return READER_ERROR;
        }

        cache_hit = add_new_block(cache, block, start, size);
    }

    off_t n = length;
    if (start + size - offset < (unsigned long) n) {
        n = start + size - offset;
    }

    memcpy(buffer, &block[offset - start], n);

    if (!cache_hit) {
        free(block);
    }

    if (length - n > 0) {
        return n + xz_read(reader, buffer + n, offset + n, length - n);
    }

    return n;
}

/* Free xz reader struct */
extern void xz_reader_free(void *reader) {
    XzReader *xz_reader = (XzReader *) reader;
    lzma_index_end(xz_reader->index, NULL);
    free_cache_entries(&xz_reader->cache);
    free(xz_reader);
}
