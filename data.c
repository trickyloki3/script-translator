#include "data.h"

int data_long_compare(void *, void *);

int kv_create(struct kv *, char *, struct json_node *, struct sector_list *);
void kv_destroy(struct kv *);

int kv_map_create(struct kv_map *, struct pool_map *);
void kv_map_destroy(struct kv_map *);
int kv_map_load(struct kv_map *, struct json_node *, struct sector_list *);

int data_kv_map_create(struct kv_map *, struct json_node *, char *, struct pool_map *, struct sector_list *);

int data_long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int kv_create(struct kv * kv, char * key, struct json_node * value, struct sector_list * sector_list) {
    int status = 0;
    char * end;

    kv->number = strtol(key, &end, 10);
    if(*end) {
        status = panic("invalid string '%s' in '%s'", end, key);
    } else {
        if(json_string_copy(value, sector_list, &kv->string))
            status = panic("failed to create char object");
    }

    return status;
}

void kv_destroy(struct kv * kv) {
    sstring_destroy(kv->string);
}

int kv_map_create(struct kv_map * kv_map, struct pool_map * pool_map) {
    int status = 0;

    kv_map->pool = pool_map_get(pool_map, sizeof(struct kv));
    if(!kv_map->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&kv_map->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&kv_map->map, data_long_compare, pool_map_get(pool_map, sizeof(struct map_node))))
                status = panic("failed to create map object");
            if(status)
                list_destroy(&kv_map->list);
        }
    }

    return status;
}

void kv_map_destroy(struct kv_map * kv_map) {
    struct kv * kv;

    kv = list_pop(&kv_map->list);
    while(kv) {
        kv_destroy(kv);
        pool_put(kv_map->pool, kv);
        kv = list_pop(&kv_map->list);
    }

    map_destroy(&kv_map->map);
    list_destroy(&kv_map->list);
}

int kv_map_load(struct kv_map * kv_map, struct json_node * node, struct sector_list * sector_list) {
    int status = 0;
    struct map_pair pair;
    struct kv * kv;

    pair = json_object_start(node);
    while(pair.key && pair.value && !status) {
        kv = pool_get(kv_map->pool);
        if(!kv) {
            status = panic("out of memory");
        } else {
            if(kv_create(kv, pair.key, pair.value, sector_list)) {
                status = panic("failed to create kv object");
            } else {
                if(list_push(&kv_map->list, kv)) {
                    status = panic("failed to push list object");
                } else {
                    if(map_insert(&kv_map->map, &kv->number, kv))
                        status = panic("failed to insert map object");
                    if(status)
                        list_pop(&kv_map->list);
                }
                if(status)
                    kv_destroy(kv);
            }
            if(status)
                pool_put(kv_map->pool, kv);
        }
        pair = json_object_next(node);
    }

    return status;
}

int data_kv_map_create(struct kv_map * kv_map, struct json_node * node, char * key, struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;
    struct json_node * object;

    object = json_object_get(node, key);
    if(!object) {
        status = panic("failed to get json object - %s", key);
    } else {
        if(kv_map_create(kv_map, pool_map)) {
            status = panic("failed to create kv map object");
        } else {
            if(kv_map_load(kv_map, object, sector_list))
                status = panic("failed to load kv map object");
            if(status)
                kv_map_destroy(kv_map);
        }
    }

    return status;
}

int data_create(struct data * data, struct pool_map * pool_map, struct sector_list * sector_list, struct json * json) {
    int status = 0;
    struct json_node * node;

    if(json_parse(json, "data.json", &node)) {
        status = panic("failed to parse json object");
    } else {
        if( data_kv_map_create(&data->ammo_type, node, "ammo_type", pool_map, sector_list) ||
            data_kv_map_create(&data->basejob, node, "basejob", pool_map, sector_list) ||
            data_kv_map_create(&data->bonus_script_flag, node, "bonus_script_flag", pool_map, sector_list) ||
            data_kv_map_create(&data->class, node, "class", pool_map, sector_list) ||
            data_kv_map_create(&data->class_group, node, "class_group", pool_map, sector_list) ||
            data_kv_map_create(&data->gender, node, "gender", pool_map, sector_list) ||
            data_kv_map_create(&data->getiteminfo_type, node, "getiteminfo_type", pool_map, sector_list) ||
            data_kv_map_create(&data->item_location, node, "item_location", pool_map, sector_list) ||
            data_kv_map_create(&data->item_type, node, "item_type", pool_map, sector_list) ||
            data_kv_map_create(&data->job, node, "job", pool_map, sector_list) ||
            data_kv_map_create(&data->job_group, node, "job_group", pool_map, sector_list) ||
            data_kv_map_create(&data->refineable, node, "refineable", pool_map, sector_list) ||
            data_kv_map_create(&data->searchstore_effect, node, "searchstore_effect", pool_map, sector_list) ||
            data_kv_map_create(&data->skill_flag, node, "skill_flag", pool_map, sector_list) ||
            data_kv_map_create(&data->strcharinfo_type, node, "strcharinfo_type", pool_map, sector_list) ||
            data_kv_map_create(&data->weapon_type, node, "weapon_type", pool_map, sector_list) )
            status = panic("failed to json create kv map object");
        json_clear(json);
    }

    return status;
}

void data_destroy(struct data * data) {
    kv_map_destroy(&data->weapon_type);
    kv_map_destroy(&data->strcharinfo_type);
    kv_map_destroy(&data->skill_flag);
    kv_map_destroy(&data->searchstore_effect);
    kv_map_destroy(&data->refineable);
    kv_map_destroy(&data->job_group);
    kv_map_destroy(&data->job);
    kv_map_destroy(&data->item_type);
    kv_map_destroy(&data->item_location);
    kv_map_destroy(&data->getiteminfo_type);
    kv_map_destroy(&data->gender);
    kv_map_destroy(&data->class_group);
    kv_map_destroy(&data->class);
    kv_map_destroy(&data->bonus_script_flag);
    kv_map_destroy(&data->basejob);
    kv_map_destroy(&data->ammo_type);
}
