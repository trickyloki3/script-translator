#ifndef heap_h
#define heap_h

#include "list.h"
#include "map.h"
#include "range.h"
#include "logic.h"

struct pool_map {
    size_t size;
    struct pool object_pool;
    struct pool range_pool;
    struct pool map_pool;
    struct map map;
};

int pool_map_create(struct pool_map *, size_t);
void pool_map_destroy(struct pool_map *);
struct pool * pool_map_get(struct pool_map *, size_t);

struct heap {
    struct pool_map pool_map;
    struct pool * list_pool;
    struct pool * map_pool;
    struct pool * range_pool;
    struct pool * logic_pool;
};

int heap_create(struct heap *, size_t);
void heap_destroy(struct heap *);
struct pool * heap_pool(struct heap *, size_t);

#endif
