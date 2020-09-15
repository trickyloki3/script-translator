#ifndef yaml_h
#define yaml_h

#include "panic.h"

enum yaml_event {
    yaml_list_start = 1,
    yaml_list_end,
    yaml_map_start,
    yaml_map_end,
    yaml_string
};

typedef int (* yaml_cb) (enum yaml_event, char *, size_t, void *);

enum yaml_type {
    yaml_sequence,
    yaml_map
};

struct yaml_node {
    enum yaml_type type;
    int scope;
    struct yaml_node * next;
    struct yaml_node * prev;
};

struct yaml_buffer {
    char * base;
    size_t size;
};

enum yaml_token {
    end_of_file = 0,
    c_sequence_entry,
    c_mapping_value,
    c_literal,
    c_folded,
    s_indent,
    s_separate_in_line,
    l_empty,
    b_break,
    nb_char,
    ns_plain_one_line,
    ns_key_one_line
};

struct yaml {
    struct yaml_node * stack;
    struct yaml_buffer * buffer;
    void * scanner;
    yaml_cb cb;
    void * arg;
    struct yaml_node * root;
    int scope;
    int scalar;
    enum yaml_token token;
    char * string;
    size_t length;
    size_t space;
};

int yaml_create(struct yaml *, size_t, size_t);
void yaml_destroy(struct yaml *);
int yaml_parse(struct yaml *, const char *, yaml_cb, void *);

#endif
