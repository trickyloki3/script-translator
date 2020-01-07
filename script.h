#ifndef script_h
#define script_h

#include "lookup.h"

struct script_node {
    struct script_node * next;
};

struct script_state {
    struct script_state * next;
};

struct script {
    struct heap * heap;
    struct lookup * lookup;
    void * scanner;
    void * parser;
    struct strbuf strbuf;
    struct store store;
    struct script_state * state;
};

int script_create(struct script *, size_t, struct heap *, struct lookup *);
void script_destroy(struct script *);
int script_translate(struct script *, struct string *);

#endif
