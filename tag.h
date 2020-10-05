#ifndef tag_h
#define tag_h

#include "map.h"

enum tag_type {
    tag_map = 0x1,
    tag_list = 0x2,
    tag_string = 0x4
};

struct tag_node {
    int scope;
    int type;
    int id;
    char * key;
    struct map map;
    struct tag_node * list;
    struct tag_node * prev;
    void * tag;
};

struct tag {
    struct pool pool;
    struct zone zone;
    struct tag_node * root;
};

int tag_create(struct tag *, size_t);
void tag_destroy(struct tag *);
void tag_clear(struct tag *);
void tag_print(struct tag *);
int tag_load(struct tag *, struct tag_node *);
struct tag_node * tag_add(struct tag_node *, char *, size_t);

#endif
