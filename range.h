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
    struct range_node * last;
    long min;
    long max;
};

typedef int (*range_cb) (struct range *, struct range *, struct range *);

int range_create(struct range *, struct pool *);
void range_destroy(struct range *);
void range_clear(struct range *);
int range_add(struct range *, long, long);
int range_remove(struct range *, long, long);
void range_print(struct range *);
int range_assign(struct range *, struct range *);
int range_or(struct range *, struct range *, struct range *);
int range_and(struct range *, struct range *, struct range *);
int range_not(struct range *, struct range *);
int range_equal(struct range *, struct range *, struct range *);
int range_not_equal(struct range *, struct range *, struct range *);
int range_lesser(struct range *, struct range *, struct range *);
int range_lesser_equal(struct range *, struct range *, struct range *);
int range_greater(struct range *, struct range *, struct range *);
int range_greater_equal(struct range *, struct range *, struct range *);
int range_plus_unary(struct range *, struct range *);
int range_minus_unary(struct range *, struct range *);
int range_bit_not(struct range *, struct range *);
int range_bit_or(struct range *, struct range *, struct range *);
int range_bit_xor(struct range *, struct range *, struct range *);
int range_bit_and(struct range *, struct range *, struct range *);
int range_bit_left(struct range *, struct range *, struct range *);
int range_bit_right(struct range *, struct range *, struct range *);
int range_plus(struct range *, struct range *, struct range *);
int range_minus(struct range *, struct range *, struct range *);
int range_multiply(struct range *, struct range *, struct range *);
int range_divide(struct range *, struct range *, struct range *);
int range_remainder(struct range *, struct range *, struct range *);
int range_increment(struct range *, struct range *);
int range_decrement(struct range *, struct range *);
int range_min(struct range *, struct range *, struct range *);
int range_max(struct range *, struct range *, struct range *);
int range_pow(struct range *, struct range *, struct range *);
#endif
