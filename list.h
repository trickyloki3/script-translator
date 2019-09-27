#ifndef list_h
#define list_h

#include "pool.h"

struct list_node {
    void * object;
    struct list_node * next;
    struct list_node * prev;
};

struct list {
    struct list_node * root;
    struct pool * pool;
};

int list_create(struct list *, struct pool *);
void list_destroy(struct list *);
int list_push(struct list *, void *);
void * list_pop(struct list *);
void list_clear(struct list *);

#endif
