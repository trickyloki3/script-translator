#ifndef db_h
#define db_h

#include "range.h"
#include "pool_map.h"
#include "sector_list.h"
#include "aux.h"
#include "csv.h"
#include "json.h"

struct item {
    long id;
    char * aegis;
    char * name;
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
    char * bonus;
    char * onequip;
    char * onunequip;
    struct list combo;
};

struct item_tbl {
    struct pool * pool;
    struct list list;
    struct map map_id;
    struct map map_name;
};

struct item_combo {
    struct array id;
    char * bonus;
};

struct item_combo_tbl {
    struct pool * pool;
    struct list list;
};

struct skill {
    long id;
    struct array range;
    long hit;
    long inf;
    struct array element;
    long nk;
    struct array splash;
    long maxlv;
    struct array hit_amount;
    char * cast_cancel;
    long cast_def_reduce_rate;
    long inf2;
    struct array max_count;
    char * type;
    struct array blow_count;
    long inf3;
    char * macro;
    char * name;
};

struct skill_tbl {
    struct pool * pool;
    struct list list;
    struct map map_id;
    struct map map_macro;
};

struct mob {
    long id;
    char * sprite;
    char * kro;
    char * iro;
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

struct mob_tbl {
    struct pool * pool;
    struct list list;
    struct map map_id;
};

struct mob_race2 {
    char * race;
    struct array id;
};

struct mob_race2_tbl {
    struct pool * pool;
    struct list list;
};

struct produce {
    long id;
    long item_id;
    long item_lv;
    long skill_id;
    long skill_lv;
    struct array material;
};

struct produce_tbl {
    struct pool * pool;
    struct list list;
    struct map map_id;
};

struct mercenary {
    long id;
    char * sprite;
    char * name;
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

struct mercenary_tbl {
    struct pool * pool;
    struct list list;
    struct map map_id;
};

struct constant {
    char * macro;
    long value;
    char * name;
    struct range range;
};

struct constant_tbl {
    struct pool * pool;
    struct list list;
    struct map map_macro;
    struct map map_announces;
    struct map map_classes;
    struct map map_effects;
    struct map map_elements;
    struct map map_gettimes;
    struct map map_itemgroups;
    struct map map_jobs;
    struct map map_locations;
    struct map map_mapflags;
    struct map map_options;
    struct map map_races;
    struct map map_readparam;
    struct map map_sc_end;
    struct map map_sc_start;
    struct map map_sizes;
    struct map map_vip_status;
    struct map map_mob_race2;
};

struct db {
    struct pool_map * pool_map;
    struct sector_list * sector_list;
    struct pool * range_node_pool;
    struct item_tbl item_tbl;
    struct item_combo_tbl item_combo_tbl;
    struct skill_tbl skill_tbl;
    struct mob_tbl mob_tbl;
    struct mob_race2_tbl mob_race2_tbl;
    struct produce_tbl produce_tbl;
    struct mercenary_tbl mercenary_tbl;
    struct constant_tbl constant_tbl;
};

int db_create(struct db *, struct pool_map *, struct sector_list *, struct csv *, struct json *);
void db_destroy(struct db *);

#endif
