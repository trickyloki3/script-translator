#ifndef stack_h
#define stack_h

#include "pool.h"

struct stack_node {
    void ** array;
    size_t count;
    struct stack_node * next;
    struct stack_node * prev;
};

struct stack {
    size_t total;
    struct pool * pool;
    struct stack_node * root;
    struct stack_node * last;
};

int stack_create(struct stack *, struct pool *);
void stack_destroy(struct stack *);
void stack_clear(struct stack *);
int stack_push(struct stack *, void *);
void * stack_pop(struct stack *);
void * stack_top(struct stack *);
void * stack_get(struct stack *, size_t);

#endif
