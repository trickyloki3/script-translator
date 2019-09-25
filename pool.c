#include "pool.h"

inline void pool_node_attach(struct pool_node *, struct pool_node *);
inline void pool_node_detach(struct pool_node *);
int pool_expand(struct pool *);

inline void pool_node_attach(struct pool_node * x, struct pool_node * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

inline void pool_node_detach(struct pool_node * x) {
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
        pool->list = NULL;
        if(stack_create(&pool->stack, sizeof(void *), 8))
            status = panic("failed to create stack object");
    }

    return status;
}

void pool_destroy(struct pool * pool) {
    void ** buffer;

    buffer = stack_pop(&pool->stack);
    while(buffer) {
        free(*buffer);
        buffer = stack_pop(&pool->stack);
    }
    stack_destroy(&pool->stack);
}

int pool_expand(struct pool * pool) {
    int status = 0;

    size_t i;
    void * buffer;

    buffer = malloc(pool->size * pool->count);
    if(!buffer) {
        status = panic("out of memory");
    } else {
        if(stack_push_value(&pool->stack, &buffer)) {
            status = panic("failed to push stack object");
        } else {
            for(i = 0; i < pool->count; i++)
                pool_put(pool, (char *) buffer + pool->size * i);
        }
        if(status)
            free(buffer);
    }

    return status;
}

void * pool_get(struct pool * pool) {
    struct pool_node * node = NULL;

    if(pool->list || !pool_expand(pool)) {
        node = pool->list;
        pool->list = (pool->list == pool->list->next) ? NULL : pool->list->next;
        pool_node_detach(node);
    }

    return node;
}

void pool_put(struct pool * pool, void * object) {
    struct pool_node * node = object;

    node->next = node;
    node->prev = node;

    if(pool->list)
        pool_node_attach(node, pool->list);

    pool->list = node;
}
