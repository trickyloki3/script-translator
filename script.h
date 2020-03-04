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

struct script {
    struct heap * heap;
    struct table * table;
    void * scanner;
    void * parser;
    struct store store;
    struct strbuf strbuf;
};

int script_create(struct script *, size_t, struct heap *, struct table *);
void script_destroy(struct script *);
int script_compile(struct script *, char *);

#endif
