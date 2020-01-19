#ifndef logic_h
#define logic_h

#include "pool.h"

enum logic_type {
    cond,
    and,
    or,
    and_or,
    not_cond
};

struct logic_node {
    enum logic_type type;
    void * data;
    struct logic_node * root;
    struct logic_node * next;
};

struct logic {
    struct pool * pool;
    struct logic_node * root;
};

int logic_create(struct logic *, struct pool *);
void logic_destroy(struct logic *);
int logic_clear(struct logic *);
int logic_copy(struct logic *, struct logic *);
int logic_push(struct logic *, enum logic_type, void *);
int logic_pop(struct logic *);
void logic_print(struct logic *);

#endif
