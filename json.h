#ifndef json_h
#define json_h

#include "pool.h"
#include "map.h"
#include "list.h"
#include "pool_map.h"
#include "sector_list.h"
#include "aux.h"

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
        double number;
        char * string;
    };
};

struct json {
    struct pool * map_node_pool;
    struct pool * list_node_pool;
    struct pool * json_node_pool;
    struct list list;
    struct list nest;
    struct json_node * root;
    struct sector_list * sector_list;
};

int json_create(struct json *, struct pool_map *, struct sector_list *);
void json_destroy(struct json *);
void json_clear(struct json *);
int json_parse(struct json *, const char *);
int json_add_node(struct json *, enum json_type, char *, size_t, struct json_node **);
int json_push_node(struct json *, struct json_node *);
int json_pop_node(struct json *, enum json_type);
int json_insert_object(struct json *, struct json_node *, struct json_node *);
int json_insert_array(struct json *, struct json_node *);

void json_node_print(struct json_node *);

#endif
