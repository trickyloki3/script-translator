#ifndef db_h
#define db_h

#include "csv.h"
#include "json.h"
#include "yaml.h"

enum type {
    list,
    map,
    string
};

struct data {
    enum type type;
    int mark;
    struct data * data;
    struct data * next;
    struct map map;
};

struct schema {
    struct pool * map_pool;
    struct pool * pool;
    struct list list;
    struct data * root;
};

int schema_create(struct schema *, struct heap *);
void schema_destroy(struct schema *);
int schema_push(struct schema *, enum type, int, char *);
void schema_pop(struct schema *);
void schema_print(struct schema *);

struct markup {
    int level;
    enum type type;
    int mark;
    char * key;
    struct markup * next;
};

int schema_markup(struct schema *, struct markup *, struct heap *);

struct parser {
    size_t size;
    struct csv csv;
    struct json json;
    struct yaml yaml;
};

int parser_create(struct parser *, size_t, struct heap *);
void parser_destroy(struct parser *);
int parser_parse(struct parser *, const char *);

#endif
