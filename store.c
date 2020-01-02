#include "store.h"

static inline struct store_node * store_node_create(struct store *);
static inline void store_node_destroy(struct store *, struct store_node *);
static inline void * store_node_alloc(struct store_node *, size_t);

static inline struct store_node * store_node_create(struct store * store) {
    struct store_node * node;

    node = pool_get(&store->pool);
    if(node) {
        node->offset = 0;
        node->length = store->pool.size - sizeof(struct store_node);
        node->buffer = (char *) node + sizeof(struct store_node);
        node->next = NULL;
    }

    return node;
}

static inline void store_node_destroy(struct store * store, struct store_node * node) {
    memset(node->buffer, 0, node->offset);
    pool_put(&store->pool, node);
}

static inline void * store_node_alloc(struct store_node * node, size_t length) {
    void * object = NULL;

    if(node && node->length >= length) {
        object = node->buffer + node->offset;
        node->offset += length;
        node->length -= length;
    }

    return object;
}

int store_create(struct store * store, size_t size) {
    int status = 0;

    if(pool_create(&store->pool, sizeof(struct store_node) + size, 1)) {
        status = panic("failed to create pool object");
    } else {
        store->root = NULL;
    }

    return status;
}

void store_destroy(struct store * store) {
    store_clear(store);
    pool_destroy(&store->pool);
}

size_t store_size(struct store * store) {
    size_t size = 0;
    struct store_node * node;

    node = store->root;
    while(node) {
        size++;
        node = node->next;
    }

    return size *= store->pool.size;
}

void store_clear(struct store * store) {
    struct store_node * node;

    while(store->root) {
        node = store->root;
        store->root = store->root->next;
        store_node_destroy(store, node);
    }
}

void * store_object(struct store * store, size_t size) {
    void * object;
    struct store_node * node;

    object = store_node_alloc(store->root, size);
    if(!object) {
        node = store_node_create(store);
        if(node) {
            object = store_node_alloc(node, size);
            if(!object) {
                store_node_destroy(store, node);
            } else {
                node->next = store->root;
                store->root = node;
            }
        }
    }

    return object;
}
