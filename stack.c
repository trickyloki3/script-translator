#include "stack.h"

int stack_create(struct stack * stack, struct pool * pool) {
    int status = 0;

    if(!pool || pool->size < sizeof(struct stack_node)) {
        status = panic("invalid pool");
    } else {
        stack->pool = pool;
        stack->root = NULL;
    }

    return status;
}

void stack_destroy(struct stack * stack) {
    struct stack_node * node;

    while(stack->root) {
        node = stack->root;
        stack->root = stack->root->next;
        pool_put(stack->pool, node);
    }
}

int stack_push(struct stack * stack, void * object) {
    int status = 0;
    struct stack_node * node;

    if(!object) {
        status = panic("invalid object");
    } else {
        node = pool_get(stack->pool);
        if(!node) {
            status = panic("out of memory");
        } else {
            node->object = object;
            node->next = stack->root;
            stack->root = node;
        }
    }

    return status;
}

void * stack_pop(struct stack * stack) {
    void * object = NULL;
    struct stack_node * node;

    if(stack->root) {
        node = stack->root;
        object = node->object;
        stack->root = stack->root->next;
        pool_put(stack->pool, node);
    }

    return object;
}

void * stack_top(struct stack * stack) {
    return stack->root ? stack->root->object : NULL;
}
