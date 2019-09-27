#include "range.h"

inline long range_min(long, long);
inline long range_max(long, long);

int range_node_create(struct range *, long, long, struct range_node **);
void range_node_destroy(struct range *, struct range_node *);
inline void range_node_attach(struct range_node *, struct range_node *);
inline void range_node_detach(struct range_node *);

inline long range_min(long x, long y) {
    return x < y ? x : y;
}

inline long range_max(long x, long y) {
    return x < y ? y : x;
}

int range_node_create(struct range * range, long min, long max, struct range_node ** result) {
    int status = 0;
    struct range_node * node;

    node = pool_get(range->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->min = min;
        node->max = max;
        node->next = node;
        node->prev = node;
        *result = node;
    }

    return status;
}

void range_node_destroy(struct range * range, struct range_node * node) {
    pool_put(range->pool, node);
}

inline void range_node_attach(struct range_node * x, struct range_node * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

inline void range_node_detach(struct range_node * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int range_create(struct range * range, struct pool * pool) {
    int status = 0;

    if(!pool) {
        status = panic("pool is zero");
    } else if(pool->size != sizeof(struct range_node)) {
        status = panic("pool is invalid");
    } else {
        range->pool = pool;
        if(range_node_create(range, 0, 0, &range->root))
            status = panic("failed to create node object");
    }

    return status;
}

int range_create_add(struct range * range, struct pool * pool, size_t min, size_t max) {
    int status = 0;

    if(range_create(range, pool)) {
        status = panic("failed to create range object");
    } else {
        if(range_add(range, min, max))
            status = panic("failed to add range object");
        if(status)
            range_destroy(range);
    }

    return status;
}

void range_destroy(struct range * range) {
    struct range_node * node;

    if(range->root) {
        while(range->root != range->root->next) {
            node = range->root->next;
            range_node_detach(node);
            range_node_destroy(range, node);
        }
        range_node_destroy(range, range->root);
    }
}

void range_print(struct range * range) {
    struct range_node * node;

    fprintf(stdout, "global:[%ld,%ld],range:", range->root->min, range->root->max);
    if(range->root == range->root->next) {
        fprintf(stdout, "empty");
    } else {
        node = range->root->next;
        while(node != range->root) {
            fprintf(stdout, "[%ld,%ld]", node->min, node->max);
            node = node->next;
        }
    }
    fprintf(stdout,"\n");
}

int range_add(struct range * range, long x, long y) {
    int status = 0;
    long min;
    long max;
    struct range_node * iter;
    struct range_node * node;

    if(x < y) {
        min = x;
        max = y;
    } else {
        min = y;
        max = x;
    }

    if(range->root == range->root->next) {
        if(range_node_create(range, min, max, &node)) {
            status = panic("failed to create node object");
        } else {
            range_node_attach(range->root, node);
            range->root->min = min;
            range->root->max = max;
        }
    } else {
        iter = range->root->next;
        while(iter->next != range->root && iter->max + 1 < min)
            iter = iter->next;

        if(iter->max + 1 < min) {
            if(range_node_create(range, min, max, &node)) {
                status = panic("failed to create node object");
            } else {
                range_node_attach(iter, node);
            }
        } else if(iter->min - 1 > max) {
            if(range_node_create(range, min, max, &node)) {
                status = panic("failed to create node object");
            } else {
                range_node_attach(node, iter);
            }
        } else {
            iter->min = range_min(iter->min, min);
            iter->max = range_max(iter->max, max);
            while(iter->next != range->root && iter->max + 1 >= iter->next->min) {
                node = iter->next;
                iter->max = range_max(iter->max, node->max);
                range_node_detach(node);
                range_node_destroy(range, node);
            }
        }

        range_get(range, &range->root->min, &range->root->max);
    }

    return status;
}

int range_remove(struct range *  range, long x, long y) {
    int status = 0;
    long min;
    long max;
    struct range_node * iter;
    struct range_node * node;

    if(x < y) {
        min = x;
        max = y;
    } else {
        min = y;
        max = x;
    }

    if(range->root != range->root->next) {
        iter = range->root->next;
        while(iter->next != range->root && (iter->max < min || iter->min > max))
            iter = iter->next;

        while(iter != range->root && iter->max >= min && iter->min <= max && !status) {
            if(iter->min < min) {
                if(iter->max > max) {
                    if(range_node_create(range, iter->min, min - 1, &node)) {
                        status = panic("failed to create node object");
                    } else {
                        range_node_attach(node, iter);
                        iter->min = max + 1;
                    }
                } else {
                    iter->max = min - 1;
                }
            } else {
                if(iter->max > max) {
                    iter->min = max + 1;
                } else {
                    node = iter;
                    iter = iter->next;
                    range_node_detach(node);
                    range_node_destroy(range, node);
                    continue;
                }
            }
            iter = iter->next;
        }

        range_get(range, &range->root->min, &range->root->max);
    }

    return status;
}

void range_get(struct range * range, long * min, long * max) {
    *min = range->root->next->min;
    *max = range->root->prev->max;
}

void range_get_absolute(struct range * range, long * min, long * max) {
    long l;
    long r;

    range_get(range, &l, &r);
    l = labs(l);
    r = labs(r);

    if(l < r) {
        *min = l;
        *max = r;
    } else {
        *min = r;
        *max = l;
    }
}

int range_copy(struct range * result, struct range * x) {
    int status = 0;
    struct range_node * node;

    if(range_create(result, x->pool)) {
        status = panic("failed to create range object");
    } else {
        node = x->root->next;
        while(node != x->root && !status) {
            if(range_add(result, node->min, node->max))
                status = panic("failed to add range object");
            node = node->next;
        }
        if(status)
            range_destroy(result);
    }

    return status;
}

int range_negative(struct range * result, struct range * x) {
    int status = 0;
    struct range_node * node;

    if(range_create(result, x->pool)) {
        status = panic("failed to create range object");
    } else {
        node = x->root->next;
        while(node != x->root && !status) {
            if(range_add(result, -node->max, -node->min))
                status = panic("failed to add range object");
            node = node->next;
        }
        if(status)
            range_destroy(result);
    }

    return status;
}

int range_bit_not(struct range * result, struct range * x) {
    int status = 0;
    struct range_node * node;

    if(range_create(result, x->pool)) {
        status = panic("failed to create range object");
    } else {
        node = x->root->next;
        while(node != x->root && !status) {
            if(range_add(result, ~node->max, ~node->min))
                status = panic("failed to add range object");
            node = node->next;
        }
        if(status)
            range_destroy(result);
    }

    return status;
}

int range_and(struct range * result, struct range * x, struct range * y) {
    int status = 0;
    struct range_node * l;
    struct range_node * r;
    struct range_node * next;
    struct range_node * last;

    if(range_create(result, x->pool)) {
        status = panic("failed to create range object");
    } else {
        l = x->root->next;
        r = y->root->next;
        last = NULL;

        while(l != x->root && r != y->root && !status) {
            if(l->min < r->min) {
                next = l;
                l = l->next;
            } else {
                next = r;
                r = r->next;
            }

            if(!last) {
                last = next;
            } else {
                if((last->min <= next->max && last->max >= next->min))
                    if(range_add(result, range_max(last->min, next->min), range_min(last->max, next->max)))
                        status = panic("failed to add range object");
                if(last->max < next->max)
                    last = next;
            }
        }

        while(l != x->root && !status) {
            next = l;
            l = l->next;

            if(!last) {
                last = next;
            } else {
                if((last->min <= next->max && last->max >= next->min))
                    if(range_add(result, range_max(last->min, next->min), range_min(last->max, next->max)))
                        status = panic("failed to add range object");
                if(last->max < next->max)
                    last = next;
            }
        }

        while(r != y->root && !status) {
            next = r;
            r = r->next;

            if(!last) {
                last = next;
            } else {
                if((last->min <= next->max && last->max >= next->min))
                    if(range_add(result, range_max(last->min, next->min), range_min(last->max, next->max)))
                        status = panic("failed to add range object");
                if(last->max < next->max)
                    last = next;
            }
        }

        if(status)
            range_destroy(result);
    }

    return status;
}

int range_or(struct range * result, struct range * x, struct range * y) {
    int status = 0;
    struct range_node * l;
    struct range_node * r;
    struct range_node * next;

    if(range_create(result, x->pool)) {
        status = panic("failed to create range object");
    } else {
        l = x->root->next;
        r = y->root->next;

        while(l != x->root && r != y->root && !status) {
            if(l->min < r->min) {
                next = l;
                l = l->next;
            } else {
                next = r;
                r = r->next;
            }

            if(range_add(result, next->min, next->max))
                status = panic("failed to add range object");
        }

        while(l != x->root && !status) {
            next = l;
            l = l->next;

            if(range_add(result, next->min, next->max))
                status = panic("failed to add range object");
        }

        while(r != y->root && !status) {
            next = r;
            r = r->next;

            if(range_add(result, next->min, next->max))
                status = panic("failed to add range object");
        }

        if(status)
            range_destroy(result);
    }

    return status;
}

int range_not(struct range * result, struct range * x) {
    int status = 0;
    struct range_node * node;
    long xmin, xmax;

    if(range_create(result, x->pool)) {
        status = panic("failed to create range object");
    } else {
        if(x->root->min < x->root->next->min && range_add(result, x->root->min, x->root->next->min - 1)) {
            status = panic("failed to add range object");
        } else if(x->root->max > x->root->prev->max && range_add(result, x->root->prev->max + 1, x->root->max)) {
            status = panic("failed to add range object");
        }

        node = x->root->next;
        while(node->next != x->root && !status) {
            if(range_add(result, node->max + 1, node->next->min - 1))
                status = panic("failed to add range object");
            node = node->next;
        }

        result->root->min = range_min(result->root->min, x->root->min);
        result->root->max = range_max(result->root->max, x->root->max);

        if(status)
            range_destroy(result);
    }

    return status;
}

int range_equality(struct range * result, struct range * x, struct range * y) {
    return range_and(result, x, y);
}

int range_inequality(struct range * result, struct range * x, struct range * y) {
    int status = 0;
    struct range_node * node;

    if(range_copy(result, x)) {
        status = panic("failed to copy range object");
    } else {
        node = y->root->next;
        while(node != y->root && !status) {
            if(range_remove(result, node->min, node->max))
                status = panic("failed to remove range object");
            node = node->next;
        }

        if(status)
            range_destroy(result);
    }

    return status;
}

int range_less(struct range * result, struct range * x, struct range * y) {
    int status = 0;
    struct range object;
    long xmin, xmax;
    long ymin, ymax;

    range_get(x, &xmin, &xmax);
    range_get(y, &ymin, &ymax);
    if(ymax - 1 >= xmin) {
        if(range_create_add(&object, x->pool, xmin, ymax - 1)) {
            status = panic("failed to create range object");
        } else {
            if(range_and(result, x, &object))
                status = panic("failed to and range object");
            range_destroy(&object);
        }
    } else {
        if(range_create(result, x->pool))
            status = panic("failed to create range object");
    }

    return status;
}

int range_less_equal(struct range * result, struct range * x, struct range * y) {
    int status = 0;
    struct range object;
    long xmin, xmax;
    long ymin, ymax;

    range_get(x, &xmin, &xmax);
    range_get(y, &ymin, &ymax);
    if(ymax >= xmin) {
        if(range_create_add(&object, x->pool, xmin, ymax)) {
            status = panic("failed to create range object");
        } else {
            if(range_and(result, x, &object))
                status = panic("failed to and range object");
            range_destroy(&object);
        }
    } else {
        if(range_create(result, x->pool))
            status = panic("failed to create range object");
    }

    return status;
}

int range_greater(struct range * result, struct range * x, struct range * y) {
    int status = 0;
    struct range object;
    long xmin, xmax;
    long ymin, ymax;

    range_get(x, &xmin, &xmax);
    range_get(y, &ymin, &ymax);
    if(xmax >= ymax + 1) {
        if(range_create_add(&object, x->pool, ymax + 1, xmax)) {
            status = panic("failed to create range object");
        } else {
            if(range_and(result, x, &object))
                status = panic("failed to and range object");
            range_destroy(&object);
        }
    } else {
        if(range_create(result, x->pool))
            status = panic("failed to create range object");
    }

    return status;
}

int range_greater_equal(struct range * result, struct range * x, struct range * y) {
    int status = 0;
    struct range object;
    long xmin, xmax;
    long ymin, ymax;

    range_get(x, &xmin, &xmax);
    range_get(y, &ymin, &ymax);
    if(xmax >= ymax) {
        if(range_create_add(&object, x->pool, ymax, xmax)) {
            status = panic("failed to create range object");
        } else {
            if(range_and(result, x, &object))
                status = panic("failed to and range object");
            range_destroy(&object);
        }
    } else {
        if(range_create(result, x->pool))
            status = panic("failed to create range object");
    }

    return status;
}

#define range_operator(name, operator) \
int range_##name(struct range * result, struct range * x, struct range * y) { \
    int status = 0;\
    struct range_node * node;\
    long ymin, ymax;\
\
    if(range_create(result, x->pool)) {\
        status = panic("failed to create range object");\
    } else {\
        range_get(y, &ymin, &ymax);\
\
        node = x->root->next;\
        while(node != x->root && !status) {\
            if(range_add(result, node->min operator ymax, node->max operator ymax))\
                status = panic("failed to add range object");\
            node = node->next;\
        }\
\
        if(status)\
            range_destroy(result);\
    }\
\
    return status;\
}

range_operator(addition, +)
range_operator(subtraction, -)
range_operator(multiplication, *)
range_operator(division, /)
range_operator(modulus, %)
range_operator(bit_right_shift, >>)
range_operator(bit_left_shift, <<)
range_operator(bit_and, &)
range_operator(bit_or, |)
range_operator(bit_xor, ^)
