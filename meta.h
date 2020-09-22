#ifndef meta_h
#define meta_h

#include "map.h"
#include "yaml.h"

enum meta_type {
    meta_map = 0x1,
    meta_list = 0x2,
    meta_string = 0x4
};

struct meta_tag {
    int scope;
    int type;
    int id;
    char * key;
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
    struct yaml yaml;
    struct meta_node * root;
};

typedef int (* meta_cb) (enum yaml_event, int, char *, size_t, void *);

int meta_create(struct meta *, size_t, size_t);
void meta_destroy(struct meta *);
void meta_clear(struct meta *);
void meta_print(struct meta *);
int meta_parse(struct meta *, struct meta_tag *, const char *, meta_cb, void *);

#endif
