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

struct script_range {
    struct range * range;
    char * string;
};

enum script_flag {
    is_logic = 0x1,
    is_stack = 0x2
};

struct script_undef {
    struct strbuf strbuf;
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
    struct stack stack;
    struct stack range;
    struct map function;
    struct script_undef undef;
};

typedef int (*script_function) (struct script *, struct script_range **);

int script_create(struct script *, size_t, struct heap *, struct table *);
void script_destroy(struct script *);
int script_compile(struct script *, char *);
int script_statement(struct script *, struct script_node *);

#endif
