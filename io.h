#ifndef IO_H_
#define IO_H_

/* Returns a pointer to allocated memory with the contents of file at `path`.
 * The size of the file gets written at `size_o`, unless it's NULL.
 * On error returns NULL pointer.
 */
unsigned char *io_read_file(const char *path, size_t *size_o);

#endif // IO_H_
