#ifndef array_h
#define array_h

#include "utility.h"

struct long_array {
    size_t count;
    long * array;
};

struct string {
    size_t length;
    char * string;
};

struct buffer {
    size_t offset;
    size_t length;
    char * buffer;
};

int buffer_create(struct buffer *, size_t);
void buffer_destroy(struct buffer *);
void buffer_clear(struct buffer *);
int buffer_putc(struct buffer *, char);
int buffer_strdup(struct buffer *, char *, size_t);

#endif
