#ifndef store_h
#define store_h

#include "pool.h"

struct store_node {
    size_t offset;
    size_t length;
    char * buffer;
    struct store_node * next;
};

struct store {
    struct pool pool;
    struct store_node * root;
};

int store_create(struct store *, size_t);
void store_destroy(struct store *);
void store_clear(struct store *);
size_t store_size(struct store *);
void * store_object(struct store *, size_t);

#endif
