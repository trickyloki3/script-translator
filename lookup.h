#ifndef lookup_h
#define lookup_h

#include "parser.h"

struct long_array {
    long * array;
    size_t count;
};

struct pet_req {
    struct string * item;
    long amount;
    struct pet_req * next;
};

struct pet_evo {
    struct string * target;
    struct pet_req * requirement;
    struct pet_evo * next;
};

struct pet {
    struct string * mob;
    struct string * tame_item;
    struct string * egg_item;
    struct string * equip_item;
    struct string * food_item;
    long fullness;
    long hungry_delay;
    long hunger_increase;
    long intimacy_start;
    long intimacy_fed;
    long intimacy_overfed;
    long intimacy_hungry;
    long intimacy_owner_die;
    long capture_rate;
    struct string * special_performance;
    long attack_rate;
    long retaliate_rate;
    long change_target_rate;
    struct string * allow_auto_feed;
    struct string * script;
    struct string * support_script;
    struct pet_evo * evolution;
};

struct pet_db {
    struct map map;
    struct store store;
    struct pet * pet;
    struct pet_evo * evo;
    struct pet_req * req;
};

int pet_db_create(struct pet_db *, size_t, struct heap *);
void pet_db_destroy(struct pet_db *);
void pet_db_clear(struct pet_db *);
int pet_db_parse(enum parser_event, int, struct string *, void *);

struct item {
    long id;
    struct string * aegis;
    struct string * name;
    long type;
    long buy;
    long sell;
    long weight;
    long atk;
    long matk;
    long def;
    long range;
    long slots;
    unsigned long job;
    unsigned long upper;
    long gender;
    unsigned long location;
    long weapon_level;
    long base_level;
    long max_level;
    long refineable;
    long view;
    struct string * script;
};

struct item_db {
    struct map map_id;
    struct map map_aegis;
    struct store store;
    struct item * item;
    size_t index;
};

int item_db_create(struct item_db *, size_t, struct heap *);
void item_db_destroy(struct item_db *);
void item_db_clear(struct item_db *);
int item_db_parse(enum parser_event, int, struct string *, void *);

struct lookup {
    struct schema schema;
    struct parser parser;
    struct pet_db pet_db;
    struct item_db item_db;
};

int lookup_create(struct lookup *, size_t, struct heap *);
void lookup_destroy(struct lookup *);
int lookup_pet_db_parse(struct lookup *, char *);
int lookup_item_db_parse(struct lookup *, char *);

#endif
