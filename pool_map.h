#ifndef pool_map_h
#define pool_map_h

#include "pool.h"
#include "list.h"
#include "map.h"

struct pool_map {
    size_t granularity;
    struct pool list_pool;
    struct pool map_pool;
    struct list list;
    struct map map;
};

int pool_map_create(struct pool_map *, size_t);
void pool_map_destroy(struct pool_map *);
int pool_map_get(struct pool_map *, size_t, struct pool **);

#endif
