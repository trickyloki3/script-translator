#ifndef array_h
#define array_h

#include "utility.h"

struct array {
    size_t size;
    size_t count;
    void * buffer;
};

int array_create(struct array *, size_t, size_t);
void array_destroy(struct array *);
int array_expand(struct array *);
void * array_index(struct array *, size_t);

struct stack {
    size_t top;
    struct array array;
};

int stack_create(struct stack *, size_t, size_t);
void stack_destroy(struct stack *);
int stack_push_value(struct stack *, void *);
void * stack_push_reference(struct stack *);
void * stack_pop(struct stack *);
void stack_clear(struct stack *);

struct string {
    size_t offset;
    size_t length;
    char * string;
};

int string_create(struct string *, size_t);
void string_destroy(struct string *);
int string_expand(struct string *, size_t);
int string_putc(struct string *, char);
int string_strdup(struct string *, char *, size_t);
void string_clear(struct string *);

#endif
