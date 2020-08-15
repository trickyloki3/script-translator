#ifndef pool_h
#define pool_h

#include "panic.h"

struct pool_node {
    struct pool_node * next;
};

struct pool_buffer {
    char * buffer;
    struct pool_buffer * next;
};

struct pool {
    size_t size;
    size_t count;
    struct pool_node * root;
    struct pool_buffer * buffer;
};

int pool_create(struct pool *, size_t, size_t);
void pool_destroy(struct pool *);
void * pool_get(struct pool *);
void pool_put(struct pool *, void *);

#endif
