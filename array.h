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

struct strbuf {
    char * buf;
    char * str;
    char * pos;
    char * end;
    struct strbuf * next;
};

int strbuf_create(struct strbuf *, size_t);
void strbuf_destroy(struct strbuf *);
void strbuf_clear(struct strbuf *);
void strbuf_clear_move(struct strbuf *, char *, size_t);
int strbuf_putc(struct strbuf *, char);
int strbuf_putcn(struct strbuf *, char, size_t);
int strbuf_strcpy(struct strbuf *, char *, size_t);
struct string * strbuf_string(struct strbuf *);

struct buffer {
    size_t offset;
    size_t length;
    char * buffer;
    struct buffer * next;
};

int buffer_create(struct buffer *, size_t);
void buffer_destroy(struct buffer *);
void buffer_clear(struct buffer *);
void * buffer_alloc(struct buffer *, size_t);

#endif
