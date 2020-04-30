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
    int state;
    enum schema_type type;
    int mark;
    struct map * map;
    struct schema_node * list;
    struct schema_node * next;
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

void schema_push(struct schema *, struct schema_node *, int);
struct schema_node * schema_add(struct schema *, enum schema_type, int, char *);
struct schema_node * schema_get(struct schema *, char *);

struct schema_markup {
    int level;
    enum schema_type type;
    int mark;
    char * key;
};

int schema_reload(struct schema *, struct schema_markup *);
int schema_update(struct schema *, struct schema_markup *);

struct parser {
    struct yaml yaml;
    struct strbuf strbuf;
    struct schema schema;
};

int parser_create(struct parser *, size_t, struct heap *);
void parser_destroy(struct parser *);
int parser_file(struct parser *, struct schema_markup *, const char *, parser_cb, void *);
int parser_file2(struct parser *, struct schema_markup *, const char *, parser_cb, void *);

#endif
