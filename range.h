#ifndef range_h
#define range_h

#include "pool.h"

struct range_node {
    long min;
    long max;
    struct range_node * next;
    struct range_node * prev;
};

struct range {
    struct range_node * root;
    struct pool * pool;
};

int range_create(struct range *, struct pool *);
int range_create_add(struct range *, struct pool *, size_t, size_t);
void range_destroy(struct range *);
void range_print(struct range *);
int range_add(struct range *, long, long);
int range_remove(struct range *, long, long);
void range_get(struct range *, long *, long *);
void range_absolute_get(struct range *, long *, long *);
int range_copy(struct range *, struct range *);
int range_negative(struct range *, struct range *);
int range_bit_not(struct range *, struct range *);
int range_and(struct range *, struct range *, struct range *);
int range_or(struct range *, struct range *, struct range *);
int range_not(struct range *, struct range *);
int range_equality(struct range *, struct range *, struct range *);
int range_inequality(struct range *, struct range *, struct range *);
int range_less(struct range *, struct range *, struct range *);
int range_less_equal(struct range *, struct range *, struct range *);
int range_greater(struct range *, struct range *, struct range *);
int range_greater_equal(struct range *, struct range *, struct range *);
int range_addition(struct range *, struct range *, struct range *);
int range_subtraction(struct range *, struct range *, struct range *);
int range_multiplication(struct range *, struct range *, struct range *);
int range_division(struct range *, struct range *, struct range *);
int range_modulus(struct range *, struct range *, struct range *);
int range_bit_right_shift(struct range *, struct range *, struct range *);
int range_bit_left_shift(struct range *, struct range *, struct range *);
int range_bit_and(struct range *, struct range *, struct range *);
int range_bit_or(struct range *, struct range *, struct range *);
int range_bit_xor(struct range *, struct range *, struct range *);

#endif
