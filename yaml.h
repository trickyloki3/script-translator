#ifndef yaml_h
#define yaml_h

#include "event.h"
#include "heap.h"

enum yaml_token {
    c_sequence_entry = 1,
    c_mapping_value,
    c_literal,
    c_folded,
    s_indent,
    s_separate_in_line,
    l_empty,
    b_break,
    nb_char,
    ns_plain_one_line
};

enum yaml_type {
    yaml_sequence,
    yaml_map
};

struct yaml_node {
    enum yaml_type type;
    int scope;
    struct yaml_node * next;
};

struct yaml {
    struct pool * pool;
    struct yaml_node * root;
    struct strbuf strbuf;
    void * scanner;
    char * string;
    size_t length;
    size_t space;
    enum yaml_token token;
    int scope;
    int scalar;
    event_cb callback;
    void * context;
};

int yaml_create(struct yaml *, size_t, struct heap *);
void yaml_destroy(struct yaml *);
int yaml_parse(struct yaml *, const char *, event_cb, void *);

#endif
