#ifndef map_h
#define map_h

#include "slab.h"

enum map_color {
    black,
    red
};

struct map_kv {
    void * key;
    void * value;
};

struct map_node {
    void * key;
    void * value;
    enum map_color color;
    struct map_node * left;
    struct map_node * right;
    struct map_node * parent;
};

typedef int (* map_compare_cb) (void *, void *);

struct map {
    map_compare_cb compare;
    struct pool * pool;
    struct map_node * root;
    struct map_node * iter;
};

int map_create(struct map *, map_compare_cb, struct pool *);
void map_destroy(struct map *);
void map_clear(struct map *);
int map_copy(struct map *, struct map *);
int map_insert(struct map *, void *, void *);
int map_delete(struct map *, void *);
void * map_search(struct map *, void *);
struct map_kv map_start(struct map *);
struct map_kv map_next(struct map *);

#endif
