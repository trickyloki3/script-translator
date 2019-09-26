#include "pool_map.h"

int size_compare(void *, void *);

int size_compare(void * x, void * y) {
    size_t l = *((size_t *) x);
    size_t r = *((size_t *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int pool_map_create(struct pool_map * pool_map, size_t count) {
    int status = 0;

    if(pool_create(&pool_map->list_pool, sizeof(struct list_node), count)) {
        status = panic("failed to create pool object");
    } else {
        if(pool_create(&pool_map->map_pool, sizeof(struct map_node), count)) {
            status = panic("failed to create pool object");
        } else {
            if(list_create(&pool_map->list, &pool_map->list_pool)) {
                status = panic("failed to create list object");
            } else {
                if(map_create(&pool_map->map, size_compare, &pool_map->map_pool))
                    status = panic("failed to create map object");
                if(status)
                    list_destroy(&pool_map->list);
            }
            if(status)
                pool_destroy(&pool_map->map_pool);
        }
        if(status)
            pool_destroy(&pool_map->list_pool);
    }

    return status;
}

void pool_map_destroy(struct pool_map * pool_map) {
    struct pool * pool;

    pool = list_pop(&pool_map->list);
    while(pool) {
        pool_destroy(pool);
        free(pool);
        pool = list_pop(&pool_map->list);
    }

    map_destroy(&pool_map->map);
    list_destroy(&pool_map->list);
    pool_destroy(&pool_map->map_pool);
    pool_destroy(&pool_map->list_pool);
}

int pool_map_create_pool(struct pool_map * pool_map, size_t size, size_t count) {
    int status = 0;
    struct pool * pool;

    if(map_search(&pool_map->map, &size)) {
        /* pool of the same size exist */
    } else {
        pool = malloc(sizeof(*pool));
        if(!pool) {
            status = panic("out of memory");
        } else {
            if(pool_create(pool, size, count)) {
                status = panic("failed to create pool object");
            } else {
                if(list_push(&pool_map->list, pool)) {
                    status = panic("failed to push list object");
                } else {
                    if(map_insert(&pool_map->map, &pool->size, pool))
                        status = panic("failed to insert map object");
                    if(status)
                        list_pop(&pool_map->list);
                }
                if(status)
                    pool_destroy(pool);
            }
            if(status)
                free(pool);
        }
    }

    return status;
}

struct pool * pool_map_search_pool(struct pool_map * pool_map, size_t size) {
    return map_search(&pool_map->map, &size);
}
