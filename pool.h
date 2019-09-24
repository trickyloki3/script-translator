#ifndef pool_h
#define pool_h

#include "array.h"

struct pool_node {
    struct pool_node * next;
    struct pool_node * prev;
};

void pool_node_attach(struct pool_node *, struct pool_node *);
void pool_node_detach(struct pool_node *);

struct pool {
    size_t size;
    size_t count;
    struct pool_node * list;
    struct stack stack;
};

int pool_create(struct pool *, size_t, size_t);
void pool_destroy(struct pool *);
void * pool_get(struct pool *);
void pool_put(struct pool *, void *);

#endif
