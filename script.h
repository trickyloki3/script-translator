#ifndef script_h
#define script_h

#include "lookup.h"

struct script_node {
    int token;
    union {
        long integer;
        struct string * identifier;
    };
    struct range * range;
    char * string;
    struct script_node * node;
    struct script_node * next;
};

struct script_state {
    struct map * map;
    struct logic * logic;
    struct script_node * root;
    struct script_state * next;
};

struct script {
    struct heap * heap;
    struct lookup * lookup;
    void * scanner;
    void * parser;
    struct store store;
    struct strbuf strbuf;
    struct stack map;
    struct stack logic;
    struct stack range;
    struct script_state * state;
};

struct script_node * script_node_create(struct script *, int);
void script_node_print(struct script_node *);

int script_create(struct script *, size_t, struct heap *, struct lookup *);
void script_destroy(struct script *);
void script_clear(struct script *);
int script_translate(struct script *, struct string *);

#endif
