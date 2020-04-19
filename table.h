#ifndef table_h
#define table_h

#include "parser.h"

struct item_node {
    long id;
    char * name;
    char * bonus;
    char * equip;
    char * unequip;
};

struct item {
    struct store store;
    struct map id;
    struct map name;
    struct item_node * item;
    size_t index;
};

int item_create(struct item *, size_t, struct heap *);
void item_destroy(struct item *);
int item_parse(enum parser_event, int, struct string *, void *);
int item_script_parse(struct item *, char *);

struct skill_node {
    long id;
    char * name;
    char * description;
};

struct skill {
    struct store store;
    struct map id;
    struct map name;
    struct skill_node * skill;
};

int skill_create(struct skill *, size_t, struct heap *);
void skill_destroy(struct skill *);
int skill_parse(enum parser_event, int, struct string *, void *);

struct mob_node {
    long id;
    char * sprite;
    char * kro;
};

struct mob {
    struct store store;
    struct map id;
    struct map sprite;
    struct mob_node * mob;
    size_t index;
};

int mob_create(struct mob *, size_t, struct heap *);
void mob_destroy(struct mob *);
int mob_parse(enum parser_event, int, struct string *, void *);

struct mercenary_node {
    long id;
    char * name;
};

struct mercenary {
    struct store store;
    struct map id;
    struct mercenary_node * mercenary;
    size_t index;
};

int mercenary_create(struct mercenary *, size_t, struct heap *);
void mercenary_destroy(struct mercenary *);
int mercenary_parse(enum parser_event, int, struct string *, void *);

struct constant_node {
    char * identifier;
    long value;
    char * tag;
    struct range_node * range;
};

struct constant {
    struct store store;
    struct map identifier;
    struct constant_node * constant;
    struct range_node * range;
};

int constant_create(struct constant *, size_t, struct heap *);
void constant_destroy(struct constant *);
int constant_parse(enum parser_event, int, struct string *, void *);

enum spec_flag {
    spec_sign = 1 << 0,
    spec_string = 1 << 1,
    spec_percent = 1 << 2,
    spec_absolute = 1 << 3
};

struct spec_node {
    long flag;
    long divide;
};

struct array_node {
    long index;
    char * string;
};

struct print_node {
    char * string;
    struct print_node * next;
};

struct argument_node {
    char * identifier;
    char * handler;
    long newline;
    struct print_node * print;
    struct range_node * range;
    struct map * array;
    struct spec_node * spec;
    long index;
};

struct argument {
    struct store store;
    struct stack stack;
    struct map identifier;
    struct argument_node * argument;
    struct print_node * print;
    struct range_node * range;
    struct array_node * array;
    struct spec_node * spec;
};

int argument_create(struct argument *, size_t, struct heap *);
void argument_destroy(struct argument *);
int argument_print(struct argument *, struct string *, struct print_node **);
int argument_map(struct argument *, struct map **);
int argument_parse(enum parser_event, int, struct string *, void *);

struct table {
    struct schema schema;
    struct parser parser;
    struct item item;
    struct skill skill;
    struct mob mob;
    struct mercenary mercenary;
    struct constant constant;
    struct argument argument;
    struct argument bonus;
};

int table_create(struct table *, size_t, struct heap *);
void table_destroy(struct table *);
int table_parse(struct table *, struct schema_markup *, parser_cb, void *, char *);
int table_item_parse(struct table *, char *);
int table_skill_parse(struct table *, char *);
int table_mob_parse(struct table *, char *);
int table_mercenary_parse(struct table *, char *);
int table_constant_parse(struct table *, char *);
int table_argument_parse(struct table *, char *);
int table_bonus_parse(struct table *, char *);

struct item_node * item_start(struct table *);
struct item_node * item_next(struct table *);
struct item_node * item_id(struct table *, long);
struct item_node * item_name(struct table *, char *);

struct skill_node * skill_id(struct table *, long);
struct skill_node * skill_name(struct table *, char *);

struct mob_node * mob_id(struct table *, long);
struct mob_node * mob_sprite(struct table *, char *);

struct mercenary_node * mercenary_id(struct table *, long);

struct constant_node * constant_identifier(struct table *, char *);
struct argument_node * argument_identifier(struct table *, char *);
struct argument_node * bonus_identifier(struct table *, char *);

#endif
