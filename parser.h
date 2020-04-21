#ifndef parser_h
#define parser_h

#include "csv.h"
#include "json.h"
#include "yaml.h"

enum schema_type {
    list = 0x1,
    map = 0x2,
    string = 0x4
};

struct schema_node {
    enum schema_type type;
    int mark;
    struct map * map;
    struct schema_node * list;
};

struct schema {
    struct pool pool;
    struct store store;
    struct schema_node * root;
};

int schema_create(struct schema *, size_t);
void schema_destroy(struct schema *);
void schema_clear(struct schema *);
void schema_print(struct schema *);

struct schema_markup {
    int level;
    enum schema_type type;
    int mark;
    char * key;
    struct schema_node * node;
    struct schema_markup * next;
};

int schema_reload(struct schema *, struct schema_markup *);
int schema_update(struct schema *, struct schema_markup *);

enum parser_event {
    start,
    next,
    end
};

typedef int (* parser_cb) (enum parser_event, int, struct string *, void *);

struct parser {
    size_t size;
    struct pool * pool;
    struct csv csv;
    struct json json;
    struct yaml yaml;
    struct strbuf strbuf;
};

int parser_create(struct parser *, size_t, struct heap *);
void parser_destroy(struct parser *);
int parser_schema(struct parser *, struct schema *, const char *);
int parser_data(struct parser *, struct schema *, parser_cb, void *, const char *);

#endif
