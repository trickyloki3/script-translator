#ifndef json_h
#define json_h

#include "pool.h"
#include "map.h"
#include "list.h"
#include "pool_map.h"
#include "sector_list.h"

enum json_type {
    json_false,
    json_null,
    json_true,
    json_object,
    json_array,
    json_number,
    json_string
};

struct json_node {
    enum json_type type;
    union {
        struct map map;
        struct list list;
        long number;
        char * string;
    };
};

struct json {
    struct pool * map_node_pool;
    struct pool * list_node_pool;
    struct pool * json_node_pool;
    struct list list;
    struct list nest;
    struct sector_list * sector_list;
};

int json_create(struct json *, struct pool_map *, struct sector_list *);
void json_destroy(struct json *);
int json_parse(struct json *, const char *);

#endif
