#include "slab.h"

int pool_create(struct pool * pool, size_t size, size_t count) {
    if(size < sizeof(struct pool_node))
        return panic("invalid size");

    if(count == 0)
        return panic("invalid count");

    pool->size = size;
    pool->count = count;
    pool->root = NULL;
    pool->cache = NULL;

    return 0;
}

void pool_destroy(struct pool * pool) {
    struct pool_node * node;

    while(pool->cache) {
        node = pool->cache;
        pool->cache = node->next;
        free(node);
    }

    pool->root = NULL;
}

void * pool_add(struct pool * pool) {
    size_t i;
    char * buffer;
    struct pool_node * node;

    node = malloc(sizeof(*node) + pool->size * pool->count);
    if(node) {
        node->next = pool->cache;
        pool->cache = node;

        buffer = (char *) (node + 1);
        for(i = 0; i < pool->count; i++)
            pool_put(pool, buffer + pool->size * i);
    }

    return pool->root;
}

void * pool_get(struct pool * pool) {
    struct pool_node * node = NULL;

    if(pool->root || pool_add(pool)) {
        node = pool->root;
        pool->root = node->next;
    }

    return node;
}

void pool_put(struct pool * pool, void * object) {
    struct pool_node * node = object;
    node->next = pool->root;
    pool->root = node;
}

int zone_create(struct zone * zone, size_t size) {
    if(pool_create(&zone->pool, sizeof(struct zone_node) + size, 1))
        return panic("failed to create pool object");

    zone->size = size;
    zone->root = NULL;

    return 0;
}

void zone_destroy(struct zone * zone) {
    zone_clear(zone);
    pool_destroy(&zone->pool);
}

void zone_clear(struct zone * zone) {
    struct zone_node * node;

    while(zone->root) {
        node = zone->root;
        zone->root = node->next;
        pool_put(&zone->pool, node);
    }
}

int zone_add(struct zone * zone, size_t size) {
    struct zone_node * node;

    if(zone->size < size)
        return panic("invalid size");

    node = pool_get(&zone->pool);
    if(!node)
        return panic("out of memory");

    node->base = (void *) (node + 1);
    node->size = zone->size;
    node->next = zone->root;
    zone->root = node;

    return 0;
}

void * zone_get(struct zone * zone, size_t size) {
    void * object = NULL;

    if((zone->root && zone->root->size >= size) || !zone_add(zone, size)) {
        object = zone->root->base;
        zone->root->base += size;
        zone->root->size -= size;
    }

    return object;
}
