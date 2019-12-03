#ifndef heap_h
#define heap_h

#include "list.h"
#include "map.h"
#include "range.h"

struct pool_map {
    size_t size;
    struct pool object_pool;
    struct pool map_pool;
    struct map map;
};

int pool_map_create(struct pool_map *, size_t);
void pool_map_destroy(struct pool_map *);
struct pool * pool_map_get(struct pool_map *, size_t);

struct pool_list {
    size_t size;
    size_t hash;
    size_t count;
    struct pool ** pool;
};

int pool_list_create(struct pool_list *, size_t, size_t, struct pool_map *);
void pool_list_destroy(struct pool_list *);
void * pool_list_alloc(struct pool_list *, size_t);
void pool_list_free(struct pool_list *, void *, size_t);

struct heap {
    struct pool_map pool_map;
    struct pool_list pool_list;
    struct pool * list_pool;
    struct pool * map_pool;
    struct pool * range_pool;
};

int heap_create(struct heap *, size_t, size_t, size_t);
void heap_destroy(struct heap *);
void * heap_alloc(struct heap *, size_t);
void heap_free(struct heap *, void *, size_t);
struct pool * heap_pool(struct heap *, size_t);

#endif
