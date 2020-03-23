#ifndef script_h
#define script_h

#include "table.h"

struct script_node {
    int token;
    union {
        long integer;
        char * identifier;
    };
    struct script_node * root;
    struct script_node * next;
};

enum script_type {
    integer,
    identifier
};

struct script_range {
    enum script_type type;
    struct range * range;
    char * string;
};

#define ARRAY_TOTAL 32

struct script_array {
    void ** array;
    size_t count;
    size_t total;
};

struct script_buffer {
    size_t size;
    struct pool * pool;
    struct stack stack;
};

int script_buffer_create(struct script_buffer *, size_t, struct heap *);
void script_buffer_destroy(struct script_buffer *);
struct strbuf * script_buffer_get(struct script_buffer *);
void script_buffer_put(struct script_buffer *, struct strbuf *);

struct script_undef {
    struct store store;
    struct map map;
};

int script_undef_create(struct script_undef *, size_t, struct heap *);
void script_undef_destroy(struct script_undef *);
int script_undef_add(struct script_undef *, char *);
void script_undef_print(struct script_undef *);

struct script {
    struct heap * heap;
    struct table * table;
    void * scanner;
    void * parser;
    struct store store;
    struct strbuf strbuf;
    struct stack map;
    struct stack logic;
    struct stack range;
    struct stack array;
    struct map function;
    struct map argument;
    struct script_undef undef;
    struct script_node * root;
};

int script_create(struct script *, size_t, struct heap *, struct table *);
void script_destroy(struct script *);
int script_compile(struct script *, char *);

#endif
