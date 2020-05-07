#ifndef array_h
#define array_h

#include "utility.h"

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
int strbuf_putc(struct strbuf *, char);
int strbuf_putcn(struct strbuf *, char, size_t);
int strbuf_strcpy(struct strbuf *, char *, size_t);
int strbuf_printf(struct strbuf *, char *, ...);
int strbuf_vprintf(struct strbuf *, char *, va_list);
struct string * strbuf_string(struct strbuf *);
char * strbuf_array(struct strbuf *);

#endif
