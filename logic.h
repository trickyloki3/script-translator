#ifndef logic_h
#define logic_h

#include "array.h"
#include "heap.h"

enum logic_type {
    logic_var,
    logic_and,
    logic_or,
    logic_and_or
};

struct logic_var {
    void * data;
    struct range range;
    struct string * name;
};

struct logic_node {
    enum logic_type type;
    union {
        struct logic_var var;
        struct list list;
    };
};

struct logic {
    struct pool * pool;
    struct list list;
    struct map map;
    struct strbuf strbuf;
};

int logic_create(struct logic *, size_t, struct heap *);
void logic_destroy(struct logic *);
void logic_clear(struct logic *);
int logic_push_var(struct logic *, void *, struct string *, struct range *);
int logic_push_op(struct logic *, enum logic_type);
int logic_pop_op(struct logic *);
int logic_search(struct logic *, struct string *, struct range *);
void logic_print(struct logic *);

#endif
