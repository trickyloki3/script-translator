#ifndef slab_h
#define slab_h

#include "panic.h"

struct pool_node {
    struct pool_node * next;
};

struct pool {
    size_t size;
    size_t count;
    struct pool_node * root;
    struct pool_node * cache;
};

int pool_create(struct pool *, size_t, size_t);
void pool_destroy(struct pool *);
void * pool_get(struct pool *);
void pool_put(struct pool *, void *);

struct zone_node {
    char * base;
    size_t size;
    struct zone_node * next;
};

struct zone {
    size_t size;
    struct pool pool;
    struct zone_node * root;
};

int zone_create(struct zone *, size_t);
void zone_destroy(struct zone *);
void zone_clear(struct zone *);
void * zone_get(struct zone *, size_t);

#endif
