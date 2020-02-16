#ifndef yaml_h
#define yaml_h

#include "event.h"
#include "heap.h"

struct yaml_node {
    int type;
    size_t scope;
    struct string * key;
    struct string * value;
    struct yaml_node * child;
};

struct yaml {
    struct pool * pool;
    struct strbuf strbuf;
    struct strbuf scalar;
    struct stack list;
    size_t line;
    size_t space;
    struct yaml_node * root;
    struct yaml_node * stack;
    struct yaml_node * indent;
    event_cb callback;
    void * context;
};

int yaml_create(struct yaml *, size_t, struct heap *);
void yaml_destroy(struct yaml *);
int yaml_parse(struct yaml *, const char *, size_t, event_cb, void *);
int yaml_token(struct yaml *, int, size_t, struct string *, struct yaml_node **);
int yaml_string(struct yaml *, int, size_t, char *, size_t, struct yaml_node **);
int yaml_literal(struct yaml *, size_t);
int yaml_stack(struct yaml *, int);
int yaml_block(struct yaml *, struct yaml_node *);
int yaml_document(struct yaml *);

#endif
