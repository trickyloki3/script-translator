#include "pool.h"

int pool_create(struct pool * pool, size_t size, size_t count) {
    if(sizeof(struct pool_node) > size) {
        return panic("invalid size");
    } else if(!count) {
        return panic("invalid count");
    } else {
        pool->size = size;
        pool->count = count;
        pool->root = NULL;
        pool->cache = NULL;
    }

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
