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
};

int strbuf_create(struct strbuf *, size_t);
void strbuf_destroy(struct strbuf *);
void strbuf_clear(struct strbuf *);
void strbuf_clear_move(struct strbuf *, char *, size_t);
int strbuf_putc(struct strbuf *, char);
int strbuf_strcpy(struct strbuf *, char *, size_t);
struct string * strbuf_string(struct strbuf *);

#endif
