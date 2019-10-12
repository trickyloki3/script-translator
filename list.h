#ifndef list_h
#define list_h

#include "pool.h"

struct list_node {
    void * object;
    struct list_node * next;
    struct list_node * prev;
};

struct list {
    size_t size;
    struct pool * pool;
    struct list_node * root;
    struct list_node * iter;
};

int list_create(struct list *, struct pool *);
void list_destroy(struct list *);
int list_copy(struct list *, struct list *);
int list_push(struct list *, void *);
void * list_pop(struct list *);
void * list_start(struct list *);
void * list_next(struct list *);

#endif
