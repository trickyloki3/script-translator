#ifndef pool_map_h
#define pool_map_h

#include "pool.h"
#include "list.h"
#include "map.h"

struct pool_map {
    size_t granularity;
    struct list list;
    struct map map;
};

int pool_map_create(struct pool_map *, size_t, struct pool *, struct pool *);
void pool_map_destroy(struct pool_map *);
struct pool * pool_map_get(struct pool_map *, size_t);

#endif
