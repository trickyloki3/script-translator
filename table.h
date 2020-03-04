#ifndef table_h
#define table_h

#include "parser.h"

struct item_node {
    long id;
    char * name;
    char * bonus;
    char * equip;
    char * unequip;
};

struct item {
    struct store store;
    struct map id;
    struct map name;
    struct item_node * item;
    size_t index;
};

int item_create(struct item *, size_t, struct heap *);
void item_destroy(struct item *);
int item_parse(enum parser_event, int, struct string *, void *);
int item_script_parse(struct item *, char *);

struct table {
    struct schema schema;
    struct parser parser;
    struct item item;
};

int table_create(struct table *, size_t, struct heap *);
void table_destroy(struct table *);
int table_item_parse(struct table *, char *);

struct item_node * item_start(struct table *);
struct item_node * item_next(struct table *);
struct item_node * item_id(struct table *, long);

#endif
