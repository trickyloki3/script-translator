#include "pool.h"

static inline void pool_node_attach(struct pool_node *, struct pool_node *);
static inline void pool_node_detach(struct pool_node *);

int pool_alloc(struct pool *);

static inline void pool_node_attach(struct pool_node * x, struct pool_node * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

static inline void pool_node_detach(struct pool_node * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

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

        pool->count *= 2;
    }

    return status;
}

void * pool_get(struct pool * pool) {
    struct pool_node * node = NULL;

    if(pool->root || !pool_alloc(pool)) {
        node = pool->root;
        pool->root = (pool->root == pool->root->next) ? NULL : pool->root->next;
        pool_node_detach(node);
    }

    return node;
}

void pool_put(struct pool * pool, void * object) {
    struct pool_node * node = object;

    node->next = node;
    node->prev = node;

    if(pool->root)
        pool_node_attach(node, pool->root);

    pool->root = node;
}
