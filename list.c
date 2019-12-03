#include "list.h"

int list_node_create(struct list *, void *, struct list_node **);
void list_node_destroy(struct list *, struct list_node *);
static inline void list_node_attach(struct list_node *, struct list_node *);
static inline void list_node_detach(struct list_node *);

int list_node_create(struct list * list, void * object, struct list_node ** result) {
    int status = 0;
    struct list_node * node;

    node = list->pool ? pool_get(list->pool) : malloc(sizeof(*node));
    if(!node) {
        status = panic("out of memory");
    } else {
        node->object = object;
        node->next = node;
        node->prev = node;
        *result = node;
    }

    return status;
}

void list_node_destroy(struct list * list, struct list_node * node) {
    if(list->pool) {
        pool_put(list->pool, node);
    } else {
        free(node);
    }
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

    if(pool && pool->size != sizeof(struct list_node)) {
        status = panic("pool is invalid");
    } else {
        list->size = 0;
        list->pool = pool;
        list->root = NULL;
        list->iter = NULL;
    }

    return status;
}

void list_destroy(struct list * list) {
    struct list_node * node;

    list->size = 0;
    if(list->root) {
        while(list->root != list->root->next) {
            node = list->root->next;
            list_node_detach(node);
            list_node_destroy(list, node);
        }
        list_node_destroy(list, list->root);
        list->root = NULL;
    }
    list->iter = NULL;
}

void list_clear(struct list * list) {
    list_destroy(list);
}

int list_copy(struct list * result, struct list * copy) {
    int status = 0;
    void * object;

    if(list_create(result, copy->pool)) {
        status = panic("failed to create list object");
    } else {
        object = list_start(copy);
        while(object && !status) {
            if(list_push(result, object))
                status = panic("failed to push list object");
            object = list_next(copy);
        }

        if(status)
            list_destroy(result);
    }

    return status;
}

int list_push(struct list * list, void * object) {
    int status = 0;
    struct list_node * node;

    if(!object) {
        status = panic("object is zero");
    } else if(list_node_create(list, object, &node)) {
        status = panic("failed to create node object");
    } else {
        if(list->root)
            list_node_attach(list->root, node);

        list->root = node;
        list->size++;
    }

    return status;
}

void * list_pop(struct list * list) {
    void * object = NULL;
    struct list_node * node;

    if(list->root) {
        object = list->root->object;

        node = list->root;
        list->root = (list->root == list->root->prev) ? NULL : list->root->prev;
        list_node_detach(node);
        list_node_destroy(list, node);
        list->size--;
    }

    return object;
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
