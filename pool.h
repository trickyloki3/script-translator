#ifndef pool_h
#define pool_h

#include "array.h"

struct pool_node {
    struct pool_node * next;
    struct pool_node * prev;
};

#define pool_node_size sizeof(struct pool_node)

struct pool {
    size_t size;
    size_t count;
    size_t allot;
    struct pool_node * root;
    struct stack stack;
};

int pool_create(struct pool *, size_t, size_t);
void pool_destroy(struct pool *);
void * pool_get(struct pool *);
void pool_put(struct pool *, void *);

#endif
