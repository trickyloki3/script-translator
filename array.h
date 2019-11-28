#ifndef array_h
#define array_h

#include "utility.h"

struct array {
    size_t size;
    size_t count;
    void * buffer;
};

int array_create(struct array *, size_t, size_t);
void array_destroy(struct array *);
int array_expand(struct array *);
void * array_index(struct array *, size_t);

struct string {
    size_t offset;
    size_t length;
    char * string;
};

int string_create(struct string *, size_t);
void string_destroy(struct string *);
int string_copy(struct string *, struct string *);
int string_expand(struct string *, size_t);
int string_putc(struct string *, char);
int string_strdup(struct string *, char *, size_t);
int string_strtol(struct string *, int, long *);
int string_strtol_split(struct string *, int, int, struct array *);
int string_strtol_splitv(struct string *, int, int, ...);
int string_strtoul(struct string *, int, unsigned long *);
int string_strtod(struct string *, double *);
void string_clear(struct string *);

#endif
