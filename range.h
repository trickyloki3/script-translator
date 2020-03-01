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
int range_or(struct range *, struct range *, struct range *);
int range_and(struct range *, struct range *, struct range *);
int range_not(struct range *, struct range *);
int range_equal(struct range *, struct range *, struct range *);
int range_not_equal(struct range *, struct range *, struct range *);
int range_lesser(struct range *, struct range *, struct range *);
int range_lesser_equal(struct range *, struct range *, struct range *);
int range_greater(struct range *, struct range *, struct range *);
int range_greater_equal(struct range *, struct range *, struct range *);

#endif
