#include "range.h"

static inline long long_min(long, long);
static inline long long_max(long, long);

static inline struct range_node * range_node_create(struct range *, long, long);
static inline void range_node_destroy(struct range *, struct range_node *);

static inline long long_min(long x, long y) {
    return x < y ? x : y;
}

static inline long long_max(long x, long y) {
    return x < y ? y : x;
}

static inline struct range_node * range_node_create(struct range * range, long min, long max) {
    struct range_node * node;

    node = pool_get(range->pool);
    if(node) {
        node->min = min;
        node->max = max;
        node->next = NULL;
    }

    return node;
}

static inline void range_node_destroy(struct range * range, struct range_node * node) {
    pool_put(range->pool, node);
}

int range_create(struct range * range, struct pool * pool) {
    int status = 0;

    if(!pool || pool->size < sizeof(struct range_node)) {
        status = panic("invalid pool");
    } else {
        range->pool = pool;
        range->root = NULL;
        range->min = 0;
        range->max = 0;
    }

    return status;
}

void range_destroy(struct range * range) {
    struct range_node * node;

    while(range->root) {
        node = range->root;
        range->root = range->root->next;
        range_node_destroy(range, node);
    }
}

int range_add(struct range * range, long min, long max) {
    int status = 0;
    struct range_node * prev;
    struct range_node * iter;
    struct range_node * node;

    if(min > max) {
        status = panic("invalid min");
    } else {
        prev = NULL;
        iter = range->root;
        while(iter && iter->max + 1 < min) {
            prev = iter;
            iter = iter->next;
        }

        node = range_node_create(range, min, max);
        if(!node) {
            status = panic("failed to create range node object");
        } else {
            if(prev) {
                node->next = prev->next;
                prev->next = node;
            } else {
                node->next = range->root;
                range->root = node;
            }

            while(iter && iter->min <= node->max + 1) {
                node->min = long_min(node->min, iter->min);
                node->max = long_max(node->max, iter->max);
                node->next = iter->next;
                range_node_destroy(range, iter);
                iter = node->next;
            }

            if(!prev)
                range->min = node->min;
            if(!iter)
                range->max = node->max;
        }
    }

    return status;
}

int range_remove(struct range * range, long min, long max) {
    int status = 0;
    struct range_node * prev;
    struct range_node * iter;
    struct range_node * node;

    if(min > max) {
        status = panic("invalid min");
    } else {
        prev = NULL;
        iter = range->root;
        while(iter && iter->max < min) {
            prev = iter;
            iter = iter->next;
        }

        while(iter && iter->min <= max && !status) {
            if(iter->max > max) {
                if(iter->min < min) {
                    node = range_node_create(range, max + 1, iter->max);
                    if(!node) {
                        status = panic("failed to create node object");
                    } else {
                        node->next = iter->next;
                        iter->next = node;
                    }
                    iter->max = min - 1;
                } else {
                    iter->min = max + 1;
                }
                break;
            } else {
                if(iter->min < min) {
                    iter->max = min - 1;
                } else {
                    if(prev) {
                        prev->next = iter->next;
                        range_node_destroy(range, iter);
                        iter = prev->next;
                    } else {
                        range->root = iter->next;
                        range_node_destroy(range, iter);
                        iter = range->root;
                    }
                    continue;
                }
            }
            prev = iter;
            iter = iter->next;
        }

        if(!prev)
            range->min = iter ? iter->min : 0;
        if(!iter)
            range->max = prev ? prev->max : 0;
    }

    return status;
}

void range_print(struct range * range) {
    struct range_node * iter;

    iter = range->root;
    while(iter) {
        fprintf(stdout, "[%ld,%ld]", iter->min, iter->max);
        iter = iter->next;
    }
}
