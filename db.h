#ifndef db_h
#define db_h

#include "csv.h"
#include "json.h"
#include "yaml.h"

enum schema_type {
    list,
    map,
    string
};

struct schema_data {
    enum schema_type type;
    int mark;
    struct schema_data * data;
    struct schema_data * next;
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
    struct schema_data * root;
    struct list list;
};

int schema_create(struct schema *, struct heap *);
void schema_destroy(struct schema *);
int schema_push(struct schema *, enum schema_type, int, char *);
void schema_pop(struct schema *);
struct schema_data * schema_top(struct schema *);
struct schema_data * schema_load(struct schema *, struct schema_markup *);
void schema_print(struct schema_data *);

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
    struct schema_data * root;
    struct schema_data * data;
    parser_cb callback;
    void * context;
};

int parser_create(struct parser *, size_t, struct heap *);
void parser_destroy(struct parser *);
int parser_parse(struct parser *, const char *, struct schema_data *, parser_cb, void *);

#endif
