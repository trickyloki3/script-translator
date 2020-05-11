#ifndef parser_h
#define parser_h

#include "json.h"
#include "yaml.h"

enum schema_type {
    schema_list = 0x1,
    schema_map = 0x2,
    schema_string = 0x4
};

struct schema_node {
    int state;
    int type;
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
void schema_print(struct schema *);

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
int parser_schema_parse(struct parser *, struct schema *, const char *);
int parser_data_parse(struct parser *, struct schema *, const char *, parser_cb, void *);
int parser_file(struct parser *, struct schema_markup *, const char *, parser_cb, void *);
int parser_file2(struct parser *, struct schema_markup *, const char *, parser_cb, void *);

#endif
