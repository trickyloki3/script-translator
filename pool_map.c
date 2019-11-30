#include "pool_map.h"

int size_compare(void *, void *);

int size_compare(void * x, void * y) {
    size_t l = *((size_t *) x);
    size_t r = *((size_t *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int pool_map_create(struct pool_map * pool_map, size_t size) {
    int status = 0;

    if(!size) {
        status = panic("size is zero");
    } else {
        pool_map->size = size;
        if(pool_create(&pool_map->pool, sizeof(struct pool), pool_map->size / sizeof(struct pool))) {
            status = panic("failed to create pool object");
        } else {
            if(map_create(&pool_map->map, size_compare, NULL))
                status = panic("failed to create map object");
            if(status)
                pool_destroy(&pool_map->pool);
        }
    }

    return status;
}

void pool_map_destroy(struct pool_map * pool_map) {
    struct map_pair map_pair;

    map_pair = map_start(&pool_map->map);
    while(map_pair.key && map_pair.value) {
        pool_destroy(map_pair.value);
        pool_put(&pool_map->pool, map_pair.value);
        map_pair = map_next(&pool_map->map);
    }

    map_destroy(&pool_map->map);
    pool_destroy(&pool_map->pool);
}

struct pool * pool_map_get(struct pool_map * pool_map, size_t size) {
    int status = 0;
    struct pool * pool;

    pool = map_search(&pool_map->map, &size);
    if(!pool) {
        pool = pool_get(&pool_map->pool);
        if(!pool) {
            status = panic("out of memory");
        } else {
            if(pool_create(pool, size, pool_map->size / size)) {
                status = panic("failed to create pool object");
            } else {
                if(map_insert(&pool_map->map, &pool->size, pool))
                    status = panic("failed to insert map object");
                if(status)
                    pool_destroy(pool);
            }
            if(status)
                pool_put(&pool_map->pool, pool);
        }
    }

    return status ? NULL : pool;
}
