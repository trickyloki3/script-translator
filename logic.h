#ifndef logic_h
#define logic_h

#include "range.h"
#include "pool_map.h"
#include "sector_list.h"
#include "aux.h"

enum logic_type {
    logic_var,
    logic_and,
    logic_or,
    logic_and_or
};

struct logic_var {
    void * data;
    sstring name;
    struct range range;
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
    struct sector_list * sector_list;
};

int logic_create(struct logic *, struct pool_map *, struct sector_list *);
void logic_destroy(struct logic *);
int logic_push_var(struct logic *, void *, sstring, struct range *);
int logic_push_op(struct logic *, enum logic_type);
int logic_pop_op(struct logic *);
void logic_print(struct logic *);

#endif
