#include "io.h"

#include <stdio.h>
#include <stdlib.h>


unsigned char *io_read_file(const char *path, size_t *size_o)
{
    FILE *file = NULL;
    unsigned char *contents = NULL;

    file = fopen(path, "rb");
    if (!file)
        goto error_exit;

    if (fseek(file, 0, SEEK_END))
        goto error_exit;

    long size = ftell(file);
    if (size == -1L)
        goto error_exit;

    rewind(file);

    contents = malloc(size);

    if (!contents)
        goto error_exit;

    if (fread(contents, 1, size, file) != (size_t)size)
        goto error_exit;

    fclose(file);

    *size_o = size;
    return contents;

error_exit:
    if (file) {
        fclose(file);
    }

    free(contents);

    return NULL;
}
