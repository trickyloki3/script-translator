#ifndef pool_map_h
#define pool_map_h

#include "map.h"

struct pool_map {
    size_t size;
    struct map map;
};

int pool_map_create(struct pool_map *, size_t);
void pool_map_destroy(struct pool_map *);
struct pool * pool_map_get(struct pool_map *, size_t);

#endif
