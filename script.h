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
    struct script_range * next;
};

#define ARRAY_TOTAL 32

struct array {
    void * array[ARRAY_TOTAL];
    size_t count;
};

int array_add(struct array *, struct script_range *);
struct script_range * array_get(struct array *, size_t);

struct script_buffer {
    size_t size;
    struct pool * pool;
    struct stack stack;
};

int script_buffer_create(struct script_buffer *, size_t, struct heap *);
void script_buffer_destroy(struct script_buffer *);
struct strbuf * script_buffer_get(struct script_buffer *);
void script_buffer_put(struct script_buffer *, struct strbuf *);

struct undefined {
    struct strbuf strbuf;
    struct store store;
    struct map map;
};

int undefined_create(struct undefined *, size_t, struct heap *);
void undefined_destroy(struct undefined *);
int undefined_add(struct undefined *, char *, ...);
void undefined_print(struct undefined *);

struct script {
    struct heap * heap;
    struct table * table;
    void * scanner;
    void * parser;
    struct store store;
    struct stack map_stack;
    struct stack logic_stack;
    struct stack array_stack;
    struct map function;
    struct map argument;
    struct script_buffer buffer;
    struct undefined undefined;
    struct script_node * root;
    struct map * map;
    struct logic * logic;
    struct array * array;
    struct script_range * range;
};

int script_create(struct script *, size_t, struct heap *, struct table *);
void script_destroy(struct script *);
int script_compile(struct script *, char *);

#endif
