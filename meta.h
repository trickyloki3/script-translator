#ifndef meta_h
#define meta_h

#include "map.h"

enum meta_type {
    meta_map = 0x1,
    meta_list = 0x2,
    meta_string = 0x4
};

struct meta_node {
    int scope;
    int type;
    int id;
    char * key;
    struct map map;
    struct meta_node * list;
    struct meta_node * prev;
};

struct meta {
    struct pool pool;
    struct zone zone;
    struct meta_node * root;
};

int meta_create(struct meta *, size_t);
void meta_destroy(struct meta *);
void meta_clear(struct meta *);
void meta_print(struct meta *);

struct meta_node * meta_add(struct meta *, struct meta_node *, char *, size_t);

struct tag {
    int scope;
    int type;
    int id;
    char * key;
};

int meta_load(struct meta *, struct tag *);

#endif
