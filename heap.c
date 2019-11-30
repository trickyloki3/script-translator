#include "heap.h"

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
        if(pool_create(&pool_map->object_pool, sizeof(struct pool), pool_map->size / sizeof(struct pool))) {
            status = panic("failed to create pool object");
        } else {
            if(pool_create(&pool_map->map_pool, sizeof(struct map_node), pool_map->size / sizeof(struct map_node))) {
                status = panic("failed to create pool object");
            } else {
                if(map_create(&pool_map->map, size_compare, &pool_map->map_pool))
                    status = panic("failed to create map object");
                if(status)
                    pool_destroy(&pool_map->map_pool);
            }
            if(status)
                pool_destroy(&pool_map->object_pool);
        }
    }

    return status;
}

void pool_map_destroy(struct pool_map * pool_map) {
    struct map_pair kv;

    kv = map_start(&pool_map->map);
    while(kv.value) {
        pool_destroy(kv.value);
        pool_put(&pool_map->object_pool, kv.value);
        kv = map_next(&pool_map->map);
    }

    map_destroy(&pool_map->map);
    pool_destroy(&pool_map->map_pool);
    pool_destroy(&pool_map->object_pool);
}

struct pool * pool_map_get(struct pool_map * pool_map, size_t size) {
    int status = 0;
    struct pool * pool;

    pool = map_search(&pool_map->map, &size);
    if(!pool) {
        pool = pool_get(&pool_map->object_pool);
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
                pool_put(&pool_map->object_pool, pool);
        }
    }

    return status ? NULL : pool;
}


int pool_list_create(struct pool_list * pool_list, size_t size, size_t hash, struct pool_map * pool_map) {
    int status = 0;

    size_t i;

    if(hash < sizeof(struct pool_node)) {
        status = panic("hash is less than %zu", sizeof(struct pool_node));
    } else if(!size || size % hash) {
        status = panic("%zu is not divisible by %zu", size, hash);
    } else {
        pool_list->size = size;
        pool_list->hash = hash;
        pool_list->count = pool_list->size / pool_list->hash;
        pool_list->pool = malloc(sizeof(*pool_list->pool) * pool_list->count);
        if(!pool_list->pool) {
            status = panic("out of memory");
        } else {
            for(i = 0; i < pool_list->count && !status; i++) {
                pool_list->pool[i] = pool_map_get(pool_map, pool_list->hash + i * pool_list->hash);
                if(!pool_list->pool[i])
                    status = panic("failed to get pool map object");
            }
            if(status)
                free(pool_list->pool);
        }
    }

    return status;
}

void pool_list_destroy(struct pool_list * pool_list) {
    free(pool_list->pool);
}

void * pool_list_alloc(struct pool_list * pool_list, size_t size) {
    void * object = NULL;

    if(size && size <= pool_list->size)
        object = pool_get(pool_list->pool[(size - 1) / pool_list->hash]);

    return object;
}

void pool_list_free(struct pool_list * pool_list, void * object, size_t size) {
    if(size && size <= pool_list->size)
        pool_put(pool_list->pool[(size - 1) / pool_list->hash], object);
}

int heap_create(struct heap * heap, size_t per_alloc, size_t max_alloc, size_t hash) {
    int status = 0;

    if(pool_map_create(&heap->pool_map, per_alloc)) {
        status = panic("failed to create pool map object");
    } else {
        if(pool_list_create(&heap->pool_list, max_alloc, hash, &heap->pool_map))
            status = panic("failed to create pool list object");
        if(status)
            pool_map_destroy(&heap->pool_map);
    }

    return status;
}

void heap_destroy(struct heap * heap) {
    pool_list_destroy(&heap->pool_list);
    pool_map_destroy(&heap->pool_map);
}

void * heap_alloc(struct heap * heap, size_t size) {
    return pool_list_alloc(&heap->pool_list, size);
}

void heap_free(struct heap * heap, void * object, size_t size) {
    pool_list_free(&heap->pool_list, object, size);
}
