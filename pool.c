#include "pool.h"

int pool_alloc(struct pool *);

int pool_create(struct pool * pool, size_t size, size_t count) {
    int status = 0;

    if(sizeof(struct pool_node) > size) {
        status = panic("size is less than %zu", sizeof(struct pool_node));
    } else if(!count) {
        status = panic("count is zero");
    } else {
        pool->size = size;
        pool->count = count;
        pool->root = NULL;
        pool->buffer = NULL;
    }

    return status;
}

void pool_destroy(struct pool * pool) {
    struct pool_buffer * buffer;

    while(pool->buffer) {
        buffer = pool->buffer;
        pool->buffer = pool->buffer->next;
        free(buffer);
    }
}

int pool_alloc(struct pool * pool) {
    int status = 0;
    size_t i;
    struct pool_buffer * buffer;

    buffer = malloc(sizeof(*buffer) + pool->size * pool->count);
    if(!buffer) {
        status = panic("out of memory");
    } else {
        buffer->buffer = (char *) buffer + sizeof(*buffer);
        buffer->next = pool->buffer;
        pool->buffer = buffer;

        for(i = 0; i < pool->count; i++)
            pool_put(pool, buffer->buffer + pool->size * i);
    }

    return status;
}

void * pool_get(struct pool * pool) {
    struct pool_node * node = NULL;

    if(pool->root || !pool_alloc(pool)) {
        node = pool->root;
        pool->root = pool->root->next;
    }

    return node;
}

void pool_put(struct pool * pool, void * object) {
    struct pool_node * node = object;

    node->next = pool->root;
    pool->root = node;
}
