#ifndef store_h
#define store_h

#include "pool.h"

struct store_node {
    char * pos;
    char * end;
    struct store_node * next;
};

struct store {
    size_t size;
    struct store_node * root;
    struct store_node * cache;
};

int store_create(struct store *, size_t);
void store_destroy(struct store *);
void store_clear(struct store *);
void * store_malloc(struct store *, size_t);
void * store_calloc(struct store *, size_t);
char * store_strcpy(struct store *, char *, size_t);
char * store_printf(struct store *, char *, ...);
char * store_vprintf(struct store *, char *, va_list);

#endif
