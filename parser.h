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
    struct schema_node * list;
    struct schema_node * next;
    struct map map;
};

struct schema_markup {
    int level;
    enum schema_type type;
    int mark;
    char * key;
    struct schema_markup * next;
};

struct schema {
    struct pool * pool;
    struct schema_node * root;
};

int schema_create(struct schema *, struct heap *);
void schema_destroy(struct schema *);
void schema_clear(struct schema *);
int schema_push(struct schema *, enum schema_type, int, char *);
void schema_pop(struct schema *);
struct schema_node * schema_top(struct schema *);
int schema_load(struct schema *, struct schema_markup *);
void schema_print(struct schema *);

/*
 * parser_event and parser_node
 * implements the grammar below
 *
 *  node : string
 *       | start list end
 *       | start map end
 *
 *  list : node
 *       | list node
 *
 *  map  : string node
 *       | map string node
 */

enum parser_event {
    start,
    next,
    end
};

typedef int (* parser_cb) (enum parser_event, int, struct string *, void *);

struct parser {
    size_t size;
    struct csv csv;
    struct json json;
    struct yaml yaml;
    struct schema_node * root;
    struct schema_node * data;
    parser_cb callback;
    void * context;
};

int parser_create(struct parser *, size_t, struct heap *);
void parser_destroy(struct parser *);
int parser_parse(struct parser *, struct schema *, parser_cb, void *, const char *);

#endif
