#ifndef stack_h
#define stack_h

#include "pool.h"

struct stack_node {
    void * object;
    struct stack_node * next;
};

struct stack {
    struct pool * pool;
    struct stack_node * root;
};

int stack_create(struct stack *, struct pool *);
void stack_destroy(struct stack *);
void stack_clear(struct stack *);
int stack_push(struct stack *, void *);
void * stack_pop(struct stack *);
void * stack_top(struct stack *);

#endif
