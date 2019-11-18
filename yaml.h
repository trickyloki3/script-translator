#ifndef yaml_h
#define yaml_h

#include "pool_map.h"
#include "sector_list.h"
#include "sector_string.h"

enum yaml_type {
    yaml_sequence,
    yaml_mapping,
    yaml_scalar
};

struct yaml_node {
    enum yaml_type type;
    union {
        struct list sequence;
        struct map mapping;
        sstring scalar;
    };
};

struct yaml {
    struct pool * list_node_pool;
    struct pool * map_node_pool;
    struct pool * yaml_node_pool;
    struct list list;
    struct list nest;
    struct yaml_node * root;
    struct sector_list * sector_list;
};

int yaml_create(struct yaml *, struct pool_map *, struct sector_list *);
void yaml_destroy(struct yaml *);
void yaml_clear(struct yaml *);
int yaml_parse(struct yaml *, const char *);

#endif
