#ifndef data_h
#define data_h

#include "pool_map.h"
#include "sector_list.h"
#include "json.h"

struct kv {
    long number;
    char * string;
};

struct kv_map {
    struct pool * pool;
    struct list list;
    struct map map;
};

struct data {
    struct kv_map ammo_type;
    struct kv_map basejob;
    struct kv_map bonus_script_flag;
    struct kv_map class;
    struct kv_map class_group;
    struct kv_map gender;
    struct kv_map getiteminfo_type;
    struct kv_map item_location;
    struct kv_map item_type;
    struct kv_map job;
    struct kv_map job_group;
    struct kv_map refineable;
    struct kv_map searchstore_effect;
    struct kv_map skill_flag;
    struct kv_map strcharinfo_type;
    struct kv_map weapon_type;
};

int data_create(struct data *, struct pool_map *, struct sector_list *, struct json *);
void data_destroy(struct data *);

#endif
