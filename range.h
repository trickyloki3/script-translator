#ifndef range_h
#define range_h

#include "pool.h"

struct range_node {
    long min;
    long max;
    struct range_node * next;
};

struct range {
    struct pool * pool;
    struct range_node * root;
    long min;
    long max;
};

int range_create(struct range *, struct pool *);
void range_destroy(struct range *);
int range_add(struct range *, long, long);
int range_remove(struct range *, long, long);
void range_print(struct range *);

#endif
