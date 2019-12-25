#include "list.h"

static inline struct list_node * list_node_create(struct list *, void *);
static inline void list_node_destroy(struct list *, struct list_node *);
static inline void list_node_attach(struct list_node *, struct list_node *);
static inline void list_node_detach(struct list_node *);

static inline struct list_node * list_node_create(struct list * list, void * object) {
    struct list_node * node;

    node = pool_get(list->pool);
    if(node) {
        node->object = object;
        node->next = node;
        node->prev = node;
    }

    return node;
}

static inline void list_node_destroy(struct list * list, struct list_node * node) {
    pool_put(list->pool, node);
}

static inline void list_node_attach(struct list_node * x, struct list_node * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

static inline void list_node_detach(struct list_node * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int list_create(struct list * list, struct pool * pool) {
    int status = 0;

    if(!pool || pool->size < sizeof(struct list_node)) {
        status = panic("invalid pool");
    } else {
        list->pool = pool;
        list->root = NULL;
        list->iter = NULL;
    }

    return status;
}

void list_destroy(struct list * list) {
    list_clear(list);
}

void list_clear(struct list * list) {
    while(list_pop(list));
    list->iter = NULL;
}

int list_push(struct list * list, void * object) {
    int status = 0;
    struct list_node * node;

    if(!object) {
        status = panic("invalid object");
    } else {
        node = list_node_create(list, object);
        if(!node) {
            status = panic("failed to create node object");
        } else {
            if(list->root)
                list_node_attach(list->root, node);
            list->root = node;
        }
    }

    return status;
}

void * list_pop(struct list * list) {
    void * object = NULL;
    struct list_node * node;

    if(list->root) {
        node = list->root;
        list->root = (list->root == list->root->prev) ? NULL : list->root->prev;
        object = node->object;
        list_node_detach(node);
        list_node_destroy(list, node);
    }

    return object;
}

void * list_top(struct list * list) {
    return list->root ? list->root->object : NULL;
}

void * list_start(struct list * list) {
    list->iter = list->root ? list->root->next : NULL;
    return list_next(list);
}

void * list_next(struct list * list) {
    void * object = NULL;

    if(list->iter) {
        object = list->iter->object;
        list->iter = list->iter->next == list->root->next ? NULL : list->iter->next;
    }

    return object;
}
