#ifndef pool_h
#define pool_h

#include "array.h"

struct node {
    struct node * next;
    struct node * prev;
};

void node_attach(struct node *, struct node *);
void node_detach(struct node *);

struct pool {
    size_t size;
    size_t count;
    struct node * list;
    struct stack stack;
};

int pool_create(struct pool *, size_t, size_t);
void pool_destroy(struct pool *);
void * pool_get(struct pool *);
void pool_put(struct pool *, void *);

#endif
