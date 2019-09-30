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
    struct item * next;
};

struct item_tbl {
    struct item * root;
    struct pool * pool;
    struct map map_by_id;
    struct map map_by_name;
    struct sector_list * sector_list;
};

int item_tbl_create(struct item_tbl *, struct csv *, struct pool_map *, struct sector_list *);
void item_tbl_destroy(struct item_tbl *);

struct db {
    struct item_tbl item_tbl;
};

int db_create(struct db *, struct csv *, struct pool_map *, struct sector_list *);
void db_destroy(struct db *);

#endif
