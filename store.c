#include "store.h"

int store_alloc(struct store *);

int store_create(struct store * store, size_t size) {
    int status = 0;

    if(!size) {
        status = panic("invalid size");
    } else {
        store->size = size;
        store->root = NULL;
        store->cache = NULL;
    }

    return status;
}

void store_destroy(struct store * store) {
    struct store_node * node;

    store_clear(store);

    while(store->cache) {
        node = store->cache;
        store->cache = store->cache->next;
        free(node);
    }
}

void store_clear(struct store * store) {
    struct store_node * node;

    while(store->root) {
        node = store->root;
        store->root = store->root->next;
        node->next = store->cache;
        store->cache = node;
    }
}

int store_alloc(struct store * store) {
    int status = 0;
    struct store_node * node;

    if(store->cache) {
        node = store->cache;
        store->cache = store->cache->next;
    } else {
        node = malloc(sizeof(*node) + store->size);
    }

    if(!node) {
        status = panic("out of memory");
    } else {
        node->pos = (void *) (node + 1);
        node->end = node->pos + store->size;
        node->next = store->root;
        store->root = node;
    }

    return status;
}

void * store_object(struct store * store, size_t size) {
    int status = 0;
    void * object = NULL;

    if(store->size < size) {
        status = panic("invalid size");
    } else {
        if((!store->root || store->root->end - store->root->pos < size) && store_alloc(store)) {
            status = panic("out of memory");
        } else {
            object = store->root->pos;
            store->root->pos += size;
        }
    }

    return status ? NULL : object;
}
