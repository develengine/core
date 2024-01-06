#ifndef IO_H_
#define IO_H_

/* Returns a pointer to allocated memory with the contents of file at `path`.
 * The size of the file gets written at `size_o`, unless it's NULL.
 * On error returns NULL pointer.
 */
unsigned char *
io_read_file(const char *path, size_t *size_o);


#if defined(IO_IMPLEMENTATION)

#include <stdio.h>
#include <stdlib.h>

unsigned char *
io_read_file(const char *path, size_t *size_o)
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

#endif // defined(IO_IMPLEMENTATION)


#endif // IO_H_
