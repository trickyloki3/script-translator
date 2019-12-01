#include "array.h"

int buffer_create(struct buffer * buffer, size_t length) {
    int status = 0;

    if(!length) {
        status = panic("lenght is zero");
    } else {
        buffer->offset = 0;
        buffer->length = length;
        buffer->buffer = calloc(buffer->length, sizeof(*buffer->buffer));
        if(!buffer->buffer)
            status = panic("out of memory");
    }

    return status;
}

void buffer_destroy(struct buffer * buffer) {
    free(buffer->buffer);
}

void buffer_clear(struct buffer * buffer) {
    memset(buffer->buffer, 0, buffer->offset);
    buffer->length += buffer->offset;
    buffer->offset = 0;
}

int buffer_putc(struct buffer * buffer, char c) {
    int status = 0;

    if(buffer->length < 2) {
        status = panic("out of memory");
    } else {
        buffer->buffer[buffer->offset] = c;
        buffer->offset++;
        buffer->length--;
    }

    return status;
}

int buffer_strdup(struct buffer * buffer, char * string, size_t length) {
    int status = 0;

    if(buffer->length < length + 1) {
        status = panic("out of memory");
    } else {
        memcpy(buffer->buffer + buffer->offset, string, length);
        buffer->offset += length;
        buffer->length -= length;
    }

    return status;
}
