#include "stack.h"

int stack_create(struct stack * stack, struct pool * pool) {
    int status = 0;

    if(!pool || pool->size < sizeof(struct stack_node) + sizeof(void *)) {
        status = panic("invalid pool");
    } else {
        stack->total = (pool->size - sizeof(struct stack_node)) / sizeof(void *);
        stack->pool = pool;
        stack->root = NULL;
        stack->last = NULL;
    }

    return status;
}

void stack_destroy(struct stack * stack) {
    stack_clear(stack);
}

void stack_clear(struct stack * stack) {
    struct stack_node * node;

    stack->last = NULL;

    while(stack->root) {
        node = stack->root;
        stack->root = stack->root->next;
        pool_put(stack->pool, node);
    }
}

int stack_push(struct stack * stack, void * object) {
    struct stack_node * node;

    if(!object) {
        return panic("invalid object");
    } else {
        node = stack->last;
        if(!node || node->count >= stack->total) {
            node = pool_get(stack->pool);
            if(!node) {
                return panic("out of memory");
            } else {
                node->array = (void *) (node + 1);
                node->count = 0;
                if(stack->last) {
                    node->prev = stack->last;
                    stack->last->next = node;
                } else {
                    node->prev = NULL;
                    stack->root = node;
                }
                node->next = NULL;
                stack->last = node;
            }
        }

        node->array[node->count++] = object;
    }

    return 0;
}

void * stack_pop(struct stack * stack) {
    void * object = NULL;
    struct stack_node * node;

    node = stack->last;
    if(node) {
        object = node->array[--node->count];
        if(!node->count) {
            if(node->prev) {
                stack->last = node->prev;
                stack->last->next = NULL;
            } else {
                stack->last = NULL;
                stack->root = NULL;
            }
            pool_put(stack->pool, node);
        }
    }

    return object;
}

void * stack_top(struct stack * stack) {
    struct stack_node * node = stack->last;

    return node ? node->array[node->count - 1] : NULL;
}

void * stack_get(struct stack * stack, size_t index) {
    struct stack_node * node;

    node = stack->root;
    while(node && node->count <= index) {
        index -= node->count;
        node = node->next;
    }

    return node ? node->array[index] : NULL;
}

void * stack_start(struct stack * stack) {
    stack->index = 0;
    stack->entry = stack->root;
    return stack_next(stack);
}

void * stack_next(struct stack * stack) {
    void * object = NULL;
    struct stack_node * node;

    node = stack->entry;
    if(node) {
        object = node->array[stack->index++];
        if(stack->index >= node->count) {
            stack->index = 0;
            stack->entry = node->next;
        }
    }

    return object;
}
