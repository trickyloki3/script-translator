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

struct item_combo {
    struct long_array * id;
    struct string * script;
    struct item_combo * next;
};

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
    struct item_combo * combo;
};

struct item_db {
    struct map map_id;
    struct map map_aegis;
    struct store store;
    struct item * item;
    struct long_array * id;
    struct string * script;
    size_t index;
};

int item_db_create(struct item_db *, size_t, struct heap *);
void item_db_destroy(struct item_db *);
void item_db_clear(struct item_db *);
int item_db_parse(enum parser_event, int, struct string *, void *);
int item_combo_db_parse(enum parser_event, int, struct string *, void *);

struct skill {
    long id;
    struct long_array * range;
    long hit;
    long inf;
    struct long_array * element;
    long nk;
    struct long_array * splash;
    long maxlv;
    struct long_array * hit_amount;
    struct string * cast_cancel;
    long cast_def_reduce_rate;
    long inf2;
    struct long_array * max_count;
    struct string * type;
    struct long_array * blow_count;
    long inf3;
    struct string * macro;
    struct string * name;
};

struct skill_db {
    struct map map_id;
    struct map map_macro;
    struct store store;
    struct skill * skill;
    size_t index;
};

int skill_db_create(struct skill_db *, size_t, struct heap *);
void skill_db_destroy(struct skill_db *);
void skill_db_clear(struct skill_db *);
int skill_db_parse(enum parser_event, int, struct string *, void *);

struct mob {
    long id;
    struct string * sprite;
    struct string * kro;
    struct string * iro;
    long level;
    long hp;
    long sp;
    long exp;
    long jexp;
    long range1;
    long atk1;
    long atk2;
    long def;
    long mdef;
    long str;
    long agi;
    long vit;
    long inte;
    long dex;
    long luk;
    long range2;
    long range3;
    long scale;
    long race;
    long element;
    long mode;
    long speed;
    long adelay;
    long amotion;
    long dmotion;
    double mexp;
    long mvp_drop_id[3];
    long mvp_drop_chance[3];
    long drop_id[9];
    long drop_chance[9];
    long drop_card_id;
    long drop_card_chance;
};

struct mob_db {
    struct map map_id;
    struct store store;
    struct mob * mob;
    size_t index;
};

int mob_db_create(struct mob_db *, size_t, struct heap *);
void mob_db_destroy(struct mob_db *);
void mob_db_clear(struct mob_db *);
int mob_db_parse(enum parser_event, int, struct string *, void *);

struct mob_race {
    struct string * race;
    struct long_array * id;
};

struct mob_race_db {
    struct map map_race;
    struct store store;
    struct strbuf strbuf;
    struct mob_race * mob_race;
    size_t index;
};

int mob_race_db_create(struct mob_race_db *, size_t, struct heap *);
void mob_race_db_destroy(struct mob_race_db *);
void mob_race_db_clear(struct mob_race_db *);
int mob_race_db_parse(enum parser_event, int, struct string *, void *);

struct mercenary {
    long id;
    struct string * sprite;
    struct string * name;
    long level;
    long hp;
    long sp;
    long range1;
    long atk1;
    long atk2;
    long def;
    long mdef;
    long str;
    long agi;
    long vit;
    long ini;
    long dex;
    long luk;
    long range2;
    long range3;
    long scale;
    long race;
    long element;
    long speed;
    long adelay;
    long amotion;
    long dmotion;
};

struct mercenary_db {
    struct map map_id;
    struct store store;
    struct mercenary * mercenary;
    size_t index;
};

int mercenary_db_create(struct mercenary_db *, size_t, struct heap *);
void mercenary_db_destroy(struct mercenary_db *);
void mercenary_db_clear(struct mercenary_db *);
int mercenary_db_parse(enum parser_event, int, struct string *, void *);

struct lookup {
    struct schema schema;
    struct parser parser;
    struct pet_db pet_db;
    struct item_db item_db;
    struct skill_db skill_db;
    struct mob_db mob_db;
    struct mob_race_db mob_race_db;
    struct mercenary_db mercenary_db;
};

int lookup_create(struct lookup *, size_t, struct heap *);
void lookup_destroy(struct lookup *);
int lookup_pet_db_parse(struct lookup *, char *);
int lookup_item_db_parse(struct lookup *, char *);
int lookup_item_combo_db_parse(struct lookup *, char *);
int lookup_skill_db_parse(struct lookup *, char *);
int lookup_mob_db_parse(struct lookup *, char *);
int lookup_mob_race_db_parse(struct lookup *, char *);
int lookup_mercenary_parse(struct lookup *, char *);

#endif
