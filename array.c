#include "array.h"

int array_create(struct array * array, size_t size, size_t count) {
    int status = 0;

    if(!size) {
        status = panic("size is zero");
    } else if(!count) {
        status = panic("count is zero");
    } else {
        array->size = size;
        array->count = count;
        array->buffer = malloc(array->size * array->count);
        if(!array->buffer)
            status = panic("out of memory");
    }

    return status;
}

void array_destroy(struct array * array) {
    free(array->buffer);
}

int array_expand(struct array * array) {
    int status = 0;
    void * buffer;

    buffer = realloc(array->buffer, array->size * array->count * 2);
    if(!buffer) {
        status = panic("out of memory");
    } else {
        array->count *= 2;
        array->buffer = buffer;
    }

    return status;
}

void * array_index(struct array * array, size_t index) {
    void * element = NULL;

    if(index < array->count)
        element = (char *) array->buffer + array->size * index;

    return element;
}

int stack_create(struct stack * stack, size_t size, size_t count) {
    int status = 0;

    stack->top = 0;
    if(array_create(&stack->array, size, count))
        status = panic("failed to create stack object");

    return status;
}

void stack_destroy(struct stack * stack) {
    array_destroy(&stack->array);
}

int stack_push(struct stack * stack, void * element) {
    int status = 0;

    if(stack->top >= stack->array.count && array_expand(&stack->array)) {
        status = panic("stack is full");
    } else {
        memcpy(array_index(&stack->array, stack->top), element, stack->array.size);
        stack->top++;
    }

    return status;
}

int stack_pop(struct stack * stack) {
    int status = 0;

    if(!stack->top) {
        status = panic("stack is empty");
    } else {
        stack->top--;
    }

    return status;
}

void stack_clear(struct stack * stack) {
    stack->top = 0;
}
