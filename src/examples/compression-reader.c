#include <stdlib.h>
#include <stdio.h>

#include "../read_layer.h"

int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Usage: %s File Offset Length\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file '%s'\n", argv[1]);
        return 1;
    }

    off_t offset = atol(argv[2]);
    size_t length = atol(argv[3]);

    if (offset < 0 || length <= 0) {
        fprintf(stderr, "Error: Offset must be >= 0 and length must be > 0\n");
        fclose(file);
        return 1;
    }

    uint8_t *buffer = (uint8_t *) malloc(length);

    int64_t bytes_read = read_wrapper(file, buffer, offset, length);

    if (bytes_read != (int64_t) length) {
        fprintf(stderr,
                "Error: Number of bytes read does not match, %li != %li\n",
                bytes_read, length);
        free_read_wrapper();
        fclose(file);
        return 1;
    }

    fwrite(buffer, 1, length, stdout);

    free(buffer);
    free_read_wrapper();
    fclose(file);

    return 0;
}
