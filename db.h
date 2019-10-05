#ifndef db_h
#define db_h

#include "pool_map.h"
#include "sector_list.h"
#include "csv.h"

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

struct db {
    struct pool_map * pool_map;
    struct sector_list * sector_list;
    struct item_tbl item_tbl;
    struct item_combo_tbl item_combo_tbl;
};

int db_create(struct db *, struct pool_map *, struct sector_list *, struct csv *);
void db_destroy(struct db *);

#endif
