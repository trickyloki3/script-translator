#ifndef yaml_h
#define yaml_h

#include "array.h"
#include "heap.h"

struct yaml_node {
    int type;
    size_t scope;
    struct string * string;
    struct yaml_node * child;
};

struct yaml {
    struct pool * pool;
    struct strbuf strbuf;
    struct list list;
    struct yaml_node * root;
    struct yaml_node * indent;
};

int yaml_create(struct yaml *, size_t, struct heap *);
void yaml_destroy(struct yaml *);
int yaml_parse(struct yaml *, const char *, size_t);
struct string * yaml_string(struct yaml *, char *, size_t);
int yaml_token(struct yaml *, int, size_t, struct string *, struct yaml_node **);
int yaml_block(struct yaml *, struct yaml_node *, struct yaml_node *);
void yaml_clear(struct yaml *);

#endif
