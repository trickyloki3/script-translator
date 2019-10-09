#include "db.h"

int item_create(struct item *, struct list *, struct pool *, struct sector_list *);
void item_destroy(struct item *);

int item_tbl_create(struct item_tbl *, struct pool_map *);
void item_tbl_destroy(struct item_tbl *);
int item_tbl_add(struct item_tbl *, struct list *, struct sector_list *);
int item_tbl_add_combo(struct item_tbl *, struct item_combo *);

int item_combo_create(struct item_combo *, struct list *, struct sector_list *);
void item_combo_destroy(struct item_combo *);

int item_combo_tbl_create(struct item_combo_tbl *, struct pool_map *);
void item_combo_tbl_destroy(struct item_combo_tbl *);
int item_combo_tbl_add(struct item_combo_tbl *, struct list *, struct sector_list *);

int skill_create(struct skill *, struct list *, struct sector_list *);
void skill_destroy(struct skill *);

int skill_tbl_create(struct skill_tbl *, struct pool_map *);
void skill_tbl_destroy(struct skill_tbl *);
int skill_tbl_add(struct skill_tbl *, struct list *, struct sector_list *);

int mob_create(struct mob *, struct list *, struct sector_list *);
void mob_destroy(struct mob *);

int mob_tbl_create(struct mob_tbl *, struct pool_map *);
void mob_tbl_destroy(struct mob_tbl *);
int mob_tbl_add(struct mob_tbl *, struct list *, struct sector_list *);

int mob_race2_create(struct mob_race2 *, struct list *, struct sector_list *);
void mob_race2_destroy(struct mob_race2 *);

int mob_race2_tbl_create(struct mob_race2_tbl *, struct pool_map *);
void mob_race2_tbl_destroy(struct mob_race2_tbl *);
int mob_race2_tbl_add(struct mob_race2_tbl *, struct list *, struct sector_list *);

int produce_create(struct produce *, struct list *, struct sector_list *);
void produce_destroy(struct produce *);

int produce_tbl_create(struct produce_tbl *, struct pool_map *);
void produce_tbl_destroy(struct produce_tbl *);
int produce_tbl_add(struct produce_tbl *, struct list *, struct sector_list *);

int mercenary_create(struct mercenary *, struct list *, struct sector_list *);
void mercenary_destroy(struct mercenary *);

int mercenary_tbl_create(struct mercenary_tbl *, struct pool_map *);
void mercenary_tbl_destroy(struct mercenary_tbl *);
int mercenary_tbl_add(struct mercenary_tbl *, struct list *, struct sector_list *);

int constant_create(struct constant *, struct list *, struct pool *, struct sector_list *);
void constant_destroy(struct constant *);

int constant_tbl_create(struct constant_tbl *, struct pool_map *);
void constant_tbl_destroy(struct constant_tbl *);
int constant_tbl_json_create(struct constant_tbl *, struct json_node *);
int constant_tbl_add(struct constant_tbl *, struct list *, struct pool *, struct sector_list *);
int constant_tbl_add_map(struct constant_tbl *, struct json_node *, char *, struct map *);

int db_item_tbl_create_cb(struct list *, void *);
int db_item_tbl_create(struct db *, struct csv *);
int db_item_combo_tbl_create_cb(struct list *, void *);
int db_item_combo_tbl_create(struct db *, struct csv *);
int db_skill_tbl_create_cb(struct list *, void *);
int db_skill_tbl_create(struct db *, struct csv *);
int db_mob_tbl_create_cb(struct list *, void *);
int db_mob_tbl_create(struct db *, struct csv *);
int db_mob_race2_tbl_create_cb(struct list *, void *);
int db_mob_race2_tbl_create(struct db *, struct csv *);
int db_produce_tbl_create_cb(struct list *, void *);
int db_produce_tbl_create(struct db *, struct csv *);
int db_mercenary_tbl_create_cb(struct list *, void *);
int db_mercenary_tbl_create(struct db *, struct csv *);
int db_constant_tbl_create_cb(struct list *, void *);
int db_constant_tbl_create(struct db *, struct csv *, struct json *);

int item_create(struct item * item, struct list * record, struct pool * list_node_pool, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(item, 0, sizeof(*item));

    if(record->size < 22) {
        status = panic("row is missing columns");
    } else if(list_create(&item->combo, list_node_pool)) {
        status = panic("failed to create list object");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            switch(field) {
                case 0: status = char_create(sector_list, string, &item->onunequip); break;
                case 1: status = char_create(sector_list, string, &item->onequip); break;
                case 2: status = char_create(sector_list, string, &item->bonus); break;
                case 3: status = string_strtol(string, 10, &item->view); break;
                case 4: status = string_strtol(string, 10, &item->refineable); break;
                case 5: status = string_strtol_splitv(string, 10, ':', &item->base_level, &item->max_level, NULL); break;
                case 6: status = string_strtol(string, 10, &item->weapon_level); break;
                case 7: status = string_strtoul(string, 10, &item->location); break;
                case 8: status = string_strtol(string, 10, &item->gender); break;
                case 9: status = string_strtoul(string, 10, &item->upper); break;
                case 10: status = string_strtoul(string, 16, &item->job); break;
                case 11: status = string_strtol(string, 10, &item->slots); break;
                case 12: status = string_strtol(string, 10, &item->range); break;
                case 13: status = string_strtol(string, 10, &item->def); break;
                case 14: status = string_strtol_splitv(string, 10, ':', &item->atk, &item->matk, NULL); break;
                case 15: status = string_strtol(string, 10, &item->weight); break;
                case 16: status = string_strtol(string, 10, &item->sell); break;
                case 17: status = string_strtol(string, 10, &item->buy); break;
                case 18: status = string_strtol(string, 10, &item->type); break;
                case 19: status = char_create(sector_list, string, &item->name); break;
                case 20: status = char_create(sector_list, string, &item->aegis); break;
                case 21: status = string_strtol(string, 10, &item->id); break;
                default: status = panic("row has too many columns"); break;
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        item_destroy(item);

    return status;
}

void item_destroy(struct item * item) {
    list_destroy(&item->combo);
    char_destroy(item->onunequip);
    char_destroy(item->onequip);
    char_destroy(item->bonus);
    char_destroy(item->name);
    char_destroy(item->aegis);
}

int item_tbl_create(struct item_tbl * item_tbl, struct pool_map * pool_map) {
    int status = 0;

    item_tbl->pool = pool_map_get(pool_map, sizeof(struct item));
    if(!item_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&item_tbl->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&item_tbl->map_id, long_compare, pool_map_get(pool_map, sizeof(struct map_node)))) {
                status = panic("failed to create map object");
            } else {
                if(map_create(&item_tbl->map_name, string_compare, pool_map_get(pool_map, sizeof(struct map_node))))
                    status = panic("failed to create map object");
                if(status)
                    map_destroy(&item_tbl->map_id);
            }
            if(status)
                list_destroy(&item_tbl->list);
        }
    }

    return status;
}

void item_tbl_destroy(struct item_tbl * item_tbl) {
    struct item * item;

    item = list_pop(&item_tbl->list);
    while(item) {
        item_destroy(item);
        pool_put(item_tbl->pool, item);
        item = list_pop(&item_tbl->list);
    }

    map_destroy(&item_tbl->map_name);
    map_destroy(&item_tbl->map_id);
    list_destroy(&item_tbl->list);
}

int item_tbl_add(struct item_tbl * item_tbl, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    struct item * item;

    item = pool_get(item_tbl->pool);
    if(!item) {
    } else {
        if(item_create(item, record, item_tbl->list.pool, sector_list)) {
            status = panic("failed to item object");
        } else {
            if(list_push(&item_tbl->list, item)) {
                status = panic("failed to push list object");
            } else {
                if(map_insert(&item_tbl->map_id, &item->id, item)) {
                    status = panic("failed to insert map object");
                } else {
                    if(map_insert(&item_tbl->map_name, item->name, item))
                        status = panic("failed to insert map object");
                    if(status)
                        map_delete(&item_tbl->map_id, &item->id);
                }
                if(status)
                    list_pop(&item_tbl->list);
            }
            if(status)
                item_destroy(item);
        }
        if(status)
            pool_put(item_tbl->pool, item);
    }

    return status;
}

int item_tbl_add_combo(struct item_tbl * item_tbl, struct item_combo * item_combo) {
    int status = 0;

    size_t i;
    long * id;
    struct item * item;

    for(i = 0; i < item_combo->id.count && !status; i++) {
        id = array_index(&item_combo->id, i);
        item = map_search(&item_tbl->map_id, id);
        if(!item) {
            status = panic("failed to find item by id -- %ld", *id);
        } else if(list_push(&item->combo, item_combo)) {
            status = panic("failed to push list object");
        }
    }

    return status;
}

int item_combo_create(struct item_combo * item_combo, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(item_combo, 0, sizeof(*item_combo));

    if(record->size < 2) {
        status = panic("row is missing columns");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            switch(field) {
                case 0: status = char_create(sector_list, string, &item_combo->bonus); break;
                case 1: status = string_strtol_split(string, 10, ':', &item_combo->id); break;
                default: status = panic("row has too many columns"); break;
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        item_combo_destroy(item_combo);

    return status;
}

void item_combo_destroy(struct item_combo * item_combo) {
    char_destroy(item_combo->bonus);
    array_destroy(&item_combo->id);
}

int item_combo_tbl_create(struct item_combo_tbl * item_combo_tbl, struct pool_map * pool_map) {
    int status = 0;

    item_combo_tbl->pool = pool_map_get(pool_map, sizeof(struct item_combo));
    if(!item_combo_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&item_combo_tbl->list, pool_map_get(pool_map, sizeof(struct list_node))))
            status = panic("failed to create list object");
    }

    return status;
}

void item_combo_tbl_destroy(struct item_combo_tbl * item_combo_tbl) {
    struct item_combo * item_combo;

    item_combo = list_pop(&item_combo_tbl->list);
    while(item_combo) {
        item_combo_destroy(item_combo);
        pool_put(item_combo_tbl->pool, item_combo);
        item_combo = list_pop(&item_combo_tbl->list);
    }

    list_destroy(&item_combo_tbl->list);
}

int item_combo_tbl_add(struct item_combo_tbl * item_combo_tbl, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    struct item_combo * item_combo;

    item_combo = pool_get(item_combo_tbl->pool);
    if(!item_combo) {
        status = panic("out of memory");
    } else {
        if(item_combo_create(item_combo, record, sector_list)) {
            status = panic("failed to create item combo object");
        } else {
            if(list_push(&item_combo_tbl->list, item_combo))
                status = panic("failed to push list object");
            if(status)
                item_combo_destroy(item_combo);
        }
        if(status)
            pool_put(item_combo_tbl->pool, item_combo);
    }

    return status;
}

int skill_create(struct skill * skill, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(skill, 0, sizeof(*skill));

    if(record->size < 18) {
        status = panic("row is missing columns");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            switch(field) {
                case 0: status = char_create(sector_list, string, &skill->name); break;
                case 1: status = char_create(sector_list, string, &skill->macro); break;
                case 2: status = string_strtol(string, 16, &skill->inf3); break;
                case 3: status = string_strtol_split(string, 10, ':', &skill->blow_count); break;
                case 4: status = char_create(sector_list, string, &skill->type); break;
                case 5: status = string_strtol_split(string, 10, ':', &skill->max_count); break;
                case 6: status = string_strtol(string, 16, &skill->inf2); break;
                case 7: status = string_strtol(string, 10, &skill->cast_def_reduce_rate); break;
                case 8: status = char_create(sector_list, string, &skill->cast_cancel); break;
                case 9: status = string_strtol_split(string, 10, ':', &skill->hit_amount); break;
                case 10: status = string_strtol(string, 10, &skill->maxlv); break;
                case 11: status = string_strtol_split(string, 10, ':', &skill->splash); break;
                case 12: status = string_strtol(string, 16, &skill->nk); break;
                case 13: status = string_strtol_split(string, 10, ':', &skill->element); break;
                case 14: status = string_strtol(string, 10, &skill->inf); break;
                case 15: status = string_strtol(string, 10, &skill->hit); break;
                case 16: status = string_strtol_split(string, 10, ':', &skill->range); break;
                case 17: status = string_strtol(string, 10, &skill->id); break;
                default: status = panic("row has too many columns"); break;
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        skill_destroy(skill);

    return status;
}

void skill_destroy(struct skill * skill) {
    char_destroy(skill->name);
    char_destroy(skill->macro);
    array_destroy(&skill->blow_count);
    char_destroy(skill->type);
    array_destroy(&skill->max_count);
    char_destroy(skill->cast_cancel);
    array_destroy(&skill->hit_amount);
    array_destroy(&skill->splash);
    array_destroy(&skill->element);
    array_destroy(&skill->range);
}

int skill_tbl_create(struct skill_tbl * skill_tbl, struct pool_map * pool_map) {
    int status = 0;

    skill_tbl->pool = pool_map_get(pool_map, sizeof(struct skill));
    if(!skill_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&skill_tbl->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&skill_tbl->map_id, long_compare, pool_map_get(pool_map, sizeof(struct map_node)))) {
                status = panic("failed to create map object");
            } else {
                if(map_create(&skill_tbl->map_macro, string_compare, pool_map_get(pool_map, sizeof(struct map_node))))
                    status = panic("failed to create map object");
                if(status)
                    map_destroy(&skill_tbl->map_id);
            }
            if(status)
                list_destroy(&skill_tbl->list);
        }
    }

    return status;
}

void skill_tbl_destroy(struct skill_tbl * skill_tbl) {
    struct skill * skill;

    skill = list_pop(&skill_tbl->list);
    while(skill) {
        skill_destroy(skill);
        pool_put(skill_tbl->pool, skill);
        skill = list_pop(&skill_tbl->list);
    }

    map_destroy(&skill_tbl->map_macro);
    map_destroy(&skill_tbl->map_id);
    list_destroy(&skill_tbl->list);
}

int skill_tbl_add(struct skill_tbl * skill_tbl, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    struct skill * skill;

    skill = pool_get(skill_tbl->pool);
    if(!skill) {
        status = panic("out of memory");
    } else {
        if(skill_create(skill, record, sector_list)) {
            status = panic("failed to create skill object");
        } else {
            if(list_push(&skill_tbl->list, skill)) {
                status = panic("failed to push list object");
            } else {
                if(map_insert(&skill_tbl->map_id, &skill->id, skill)) {
                    status = panic("failed to insert map object");
                } else {
                    if(map_insert(&skill_tbl->map_macro, skill->macro, skill))
                        status = panic("failed to insert map object");
                    if(status)
                        map_delete(&skill_tbl->map_id, &skill->id);
                }
                if(status)
                    list_pop(&skill_tbl->list);
            }
            if(status)
                skill_destroy(skill);
        }
        if(status)
            pool_put(skill_tbl->pool, skill);
    }

    return status;
}

int mob_create(struct mob * mob, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(mob, 0, sizeof(*mob));

    if(record->size < 57) {
        status = panic("row is missing columns");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            switch(field) {
                case 0: status = string_strtol(string, 10, &mob->drop_card_chance); break;
                case 1: status = string_strtol(string, 10, &mob->drop_card_id); break;
                case 2: status = string_strtol(string, 10, &mob->drop_chance[8]); break;
                case 3: status = string_strtol(string, 10, &mob->drop_id[8]); break;
                case 4: status = string_strtol(string, 10, &mob->drop_chance[7]); break;
                case 5: status = string_strtol(string, 10, &mob->drop_id[7]); break;
                case 6: status = string_strtol(string, 10, &mob->drop_chance[6]); break;
                case 7: status = string_strtol(string, 10, &mob->drop_id[6]); break;
                case 8: status = string_strtol(string, 10, &mob->drop_chance[5]); break;
                case 9: status = string_strtol(string, 10, &mob->drop_id[5]); break;
                case 10: status = string_strtol(string, 10, &mob->drop_chance[4]); break;
                case 11: status = string_strtol(string, 10, &mob->drop_id[4]); break;
                case 12: status = string_strtol(string, 10, &mob->drop_chance[3]); break;
                case 13: status = string_strtol(string, 10, &mob->drop_id[3]); break;
                case 14: status = string_strtol(string, 10, &mob->drop_chance[2]); break;
                case 15: status = string_strtol(string, 10, &mob->drop_id[2]); break;
                case 16: status = string_strtol(string, 10, &mob->drop_chance[1]); break;
                case 17: status = string_strtol(string, 10, &mob->drop_id[1]); break;
                case 18: status = string_strtol(string, 10, &mob->drop_chance[0]); break;
                case 19: status = string_strtol(string, 10, &mob->drop_id[0]); break;
                case 20: status = string_strtol(string, 10, &mob->mvp_drop_chance[2]); break;
                case 21: status = string_strtol(string, 10, &mob->mvp_drop_id[2]); break;
                case 22: status = string_strtol(string, 10, &mob->mvp_drop_chance[1]); break;
                case 23: status = string_strtol(string, 10, &mob->mvp_drop_id[1]); break;
                case 24: status = string_strtol(string, 10, &mob->mvp_drop_chance[0]); break;
                case 25: status = string_strtol(string, 10, &mob->mvp_drop_id[0]); break;
                case 26: status = string_strtod(string, &mob->mexp); break;
                case 27: status = string_strtol(string, 10, &mob->dmotion); break;
                case 28: status = string_strtol(string, 10, &mob->amotion); break;
                case 29: status = string_strtol(string, 10, &mob->adelay); break;
                case 30: status = string_strtol(string, 10, &mob->speed); break;
                case 31: status = string_strtol(string, 16, &mob->mode); break;
                case 32: status = string_strtol(string, 10, &mob->element); break;
                case 33: status = string_strtol(string, 10, &mob->race); break;
                case 34: status = string_strtol(string, 10, &mob->scale); break;
                case 35: status = string_strtol(string, 10, &mob->range3); break;
                case 36: status = string_strtol(string, 10, &mob->range2); break;
                case 37: status = string_strtol(string, 10, &mob->luk); break;
                case 38: status = string_strtol(string, 10, &mob->dex); break;
                case 39: status = string_strtol(string, 10, &mob->inte); break;
                case 40: status = string_strtol(string, 10, &mob->vit); break;
                case 41: status = string_strtol(string, 10, &mob->agi); break;
                case 42: status = string_strtol(string, 10, &mob->str); break;
                case 43: status = string_strtol(string, 10, &mob->mdef); break;
                case 44: status = string_strtol(string, 10, &mob->def); break;
                case 45: status = string_strtol(string, 10, &mob->atk2); break;
                case 46: status = string_strtol(string, 10, &mob->atk1); break;
                case 47: status = string_strtol(string, 10, &mob->range1); break;
                case 48: status = string_strtol(string, 10, &mob->jexp); break;
                case 49: status = string_strtol(string, 10, &mob->exp); break;
                case 50: status = string_strtol(string, 10, &mob->sp); break;
                case 51: status = string_strtol(string, 10, &mob->hp); break;
                case 52: status = string_strtol(string, 10, &mob->level); break;
                case 53: status = char_create(sector_list, string, &mob->iro); break;
                case 54: status = char_create(sector_list, string, &mob->kro); break;
                case 55: status = char_create(sector_list, string, &mob->sprite); break;
                case 56: status = string_strtol(string, 10, &mob->id); break;
                default: status = panic("row has too many columns"); break;
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        mob_destroy(mob);

    return status;
}

void mob_destroy(struct mob * mob) {
    char_destroy(mob->iro);
    char_destroy(mob->kro);
    char_destroy(mob->sprite);
}

int mob_tbl_create(struct mob_tbl * mob_tbl, struct pool_map * pool_map) {
    int status = 0;

    mob_tbl->pool = pool_map_get(pool_map, sizeof(struct mob));
    if(!mob_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&mob_tbl->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&mob_tbl->map_id, long_compare, pool_map_get(pool_map, sizeof(struct map_node))))
                status = panic("failed to create map object");
            if(status)
                list_destroy(&mob_tbl->list);
        }
    }

    return status;
}

void mob_tbl_destroy(struct mob_tbl * mob_tbl) {
    struct mob * mob;

    mob = list_pop(&mob_tbl->list);
    while(mob) {
        mob_destroy(mob);
        pool_put(mob_tbl->pool, mob);
        mob = list_pop(&mob_tbl->list);
    }

    map_destroy(&mob_tbl->map_id);
    list_destroy(&mob_tbl->list);
}

int mob_tbl_add(struct mob_tbl * mob_tbl, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    struct mob * mob;

    mob = pool_get(mob_tbl->pool);
    if(!mob) {
        status = panic("out of memory");
    } else {
        if(mob_create(mob, record, sector_list)) {
            status = panic("failed to create mob object");
        } else {
            if(list_push(&mob_tbl->list, mob)) {
                status = panic("failed to push list object");
            } else {
                if(map_insert(&mob_tbl->map_id, &mob->id, mob))
                    status = panic("failed to insert map object");
                if(status)
                    list_pop(&mob_tbl->list);
            }
            if(status)
                mob_destroy(mob);
        }
        if(status)
            pool_put(mob_tbl->pool, mob);
    }

    return status;
}

int mob_race2_create(struct mob_race2 * mob_race2, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(mob_race2, 0, sizeof(*mob_race2));

    if(record->size < 2) {
        status = panic("row is missing columns");
    } else if(array_create(&mob_race2->id, sizeof(long), record->size - 1)) {
        status = panic("failed to create array object");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            if(field < record->size - 1) {
                status = string_strtol(string, 10, array_index(&mob_race2->id, field));
            } else if(field == record->size - 1) {
                status = char_create(sector_list, string, &mob_race2->race);
            } else {
                status = panic("row has too many columns");
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        mob_race2_destroy(mob_race2);

    return status;
}

void mob_race2_destroy(struct mob_race2 * mob_race2) {
    array_destroy(&mob_race2->id);
    char_destroy(mob_race2->race);
}

int mob_race2_tbl_create(struct mob_race2_tbl * mob_race2_tbl, struct pool_map * pool_map) {
    int status = 0;

    mob_race2_tbl->pool = pool_map_get(pool_map, sizeof(struct mob_race2));
    if(!mob_race2_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&mob_race2_tbl->list, pool_map_get(pool_map, sizeof(struct list_node))))
            status = panic("failed to create list object");
    }

    return status;
}

void mob_race2_tbl_destroy(struct mob_race2_tbl * mob_race2_tbl) {
    struct mob_race2 * mob_race2;

    mob_race2 = list_pop(&mob_race2_tbl->list);
    while(mob_race2) {
        mob_race2_destroy(mob_race2);
        pool_put(mob_race2_tbl->pool, mob_race2);
        mob_race2 = list_pop(&mob_race2_tbl->list);
    }

    list_destroy(&mob_race2_tbl->list);
}

int mob_race2_tbl_add(struct mob_race2_tbl * mob_race2_tbl, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    struct mob_race2 * mob_race2;

    mob_race2 = pool_get(mob_race2_tbl->pool);
    if(!mob_race2) {
        status = panic("out of memory");
    } else {
        if(mob_race2_create(mob_race2, record, sector_list)) {
            status = panic("failed to create mob race 2 object");
        } else {
            if(list_push(&mob_race2_tbl->list, mob_race2))
                status = panic("failed to push list object");
            if(status)
                mob_race2_destroy(mob_race2);
        }
        if(status)
            pool_put(mob_race2_tbl->pool, mob_race2);
    }

    return status;
}

int produce_create(struct produce * produce, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(produce, 0, sizeof(*produce));

    if(record->size < 7 || (record->size - 5) % 2 != 0) {
        status = panic("row is missing columns");
    } else if(array_create(&produce->material, sizeof(long), record->size - 5)) {
        status = panic("failed to create array object");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            if(field < record->size - 5) {
                status = string_strtol(string, 10, array_index(&produce->material, field));
            } else {
                switch(record->size - field) {
                    case 5: string_strtol(string, 10, &produce->skill_lv); break;
                    case 4: string_strtol(string, 10, &produce->skill_id); break;
                    case 3: string_strtol(string, 10, &produce->item_lv); break;
                    case 2: string_strtol(string, 10, &produce->item_id); break;
                    case 1: string_strtol(string, 10, &produce->id); break;
                    default: status = panic("row has too many columns"); break;
                }
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        produce_destroy(produce);

    return status;
}

void produce_destroy(struct produce * produce) {
    array_destroy(&produce->material);
}

int produce_tbl_create(struct produce_tbl * produce_tbl, struct pool_map * pool_map) {
    int status = 0;

    produce_tbl->pool = pool_map_get(pool_map, sizeof(struct produce));
    if(!produce_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&produce_tbl->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&produce_tbl->map_id, long_compare, pool_map_get(pool_map, sizeof(struct map_node))))
                status = panic("failed to create map object");
            if(status)
                list_destroy(&produce_tbl->list);
        }
    }

    return status;
}

void produce_tbl_destroy(struct produce_tbl * produce_tbl) {
    struct produce * produce;

    produce = list_pop(&produce_tbl->list);
    while(produce) {
        produce_destroy(produce);
        pool_put(produce_tbl->pool, produce);
        produce = list_pop(&produce_tbl->list);
    }

    map_destroy(&produce_tbl->map_id);
    list_destroy(&produce_tbl->list);
}

int produce_tbl_add(struct produce_tbl * produce_tbl, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    struct produce * produce;

    produce = pool_get(produce_tbl->pool);
    if(!produce) {
        status = panic("out of memory");
    } else {
        if(produce_create(produce, record, sector_list)) {
            status = panic("failed to create produce object");
        } else {
            if(list_push(&produce_tbl->list, produce)) {
                status = panic("failed to push list object");
            } else {
                if(map_insert(&produce_tbl->map_id, &produce->id, produce))
                    status = panic("failed to insert map object");
                if(status)
                    list_pop(&produce_tbl->list);
            }
            if(status)
                produce_destroy(produce);
        }
        if(status)
            pool_put(produce_tbl->pool, produce);
    }

    return status;
}

int mercenary_create(struct mercenary * mercenary, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(mercenary, 0, sizeof(*mercenary));

    if(record->size < 26) {
        status = panic("row is missing columns");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            switch(field) {
                case 0: status = string_strtol(string, 10, &mercenary->dmotion); break;
                case 1: status = string_strtol(string, 10, &mercenary->amotion); break;
                case 2: status = string_strtol(string, 10, &mercenary->adelay); break;
                case 3: status = string_strtol(string, 10, &mercenary->speed); break;
                case 4: status = string_strtol(string, 10, &mercenary->element); break;
                case 5: status = string_strtol(string, 10, &mercenary->race); break;
                case 6: status = string_strtol(string, 10, &mercenary->scale); break;
                case 7: status = string_strtol(string, 10, &mercenary->range3); break;
                case 8: status = string_strtol(string, 10, &mercenary->range2); break;
                case 9: status = string_strtol(string, 10, &mercenary->luk); break;
                case 10: status = string_strtol(string, 10, &mercenary->dex); break;
                case 11: status = string_strtol(string, 10, &mercenary->ini); break;
                case 12: status = string_strtol(string, 10, &mercenary->vit); break;
                case 13: status = string_strtol(string, 10, &mercenary->agi); break;
                case 14: status = string_strtol(string, 10, &mercenary->str); break;
                case 15: status = string_strtol(string, 10, &mercenary->mdef); break;
                case 16: status = string_strtol(string, 10, &mercenary->def); break;
                case 17: status = string_strtol(string, 10, &mercenary->atk2); break;
                case 18: status = string_strtol(string, 10, &mercenary->atk1); break;
                case 19: status = string_strtol(string, 10, &mercenary->range1); break;
                case 20: status = string_strtol(string, 10, &mercenary->sp); break;
                case 21: status = string_strtol(string, 10, &mercenary->hp); break;
                case 22: status = string_strtol(string, 10, &mercenary->level); break;
                case 23: status = char_create(sector_list, string, &mercenary->name); break;
                case 24: status = char_create(sector_list, string, &mercenary->sprite); break;
                case 25: status = string_strtol(string, 10, &mercenary->id); break;
                default: status = panic("row has too many columns"); break;
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        mercenary_destroy(mercenary);

    return status;
}

void mercenary_destroy(struct mercenary * mercenary) {
    char_destroy(mercenary->name);
    char_destroy(mercenary->sprite);
}

int mercenary_tbl_create(struct mercenary_tbl * mercenary_tbl, struct pool_map * pool_map) {
    int status = 0;

    mercenary_tbl->pool = pool_map_get(pool_map, sizeof(struct mercenary));
    if(!mercenary_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&mercenary_tbl->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&mercenary_tbl->map_id, long_compare, pool_map_get(pool_map, sizeof(struct map_node))))
                status = panic("failed to create map object");
            if(status)
                list_destroy(&mercenary_tbl->list);
        }
    }

    return status;
}

void mercenary_tbl_destroy(struct mercenary_tbl * mercenary_tbl) {
    struct mercenary * mercenary;

    mercenary = list_pop(&mercenary_tbl->list);
    while(mercenary) {
        mercenary_destroy(mercenary);
        pool_put(mercenary_tbl->pool, mercenary);
        mercenary = list_pop(&mercenary_tbl->list);
    }

    map_destroy(&mercenary_tbl->map_id);
    list_destroy(&mercenary_tbl->list);
}

int mercenary_tbl_add(struct mercenary_tbl * mercenary_tbl, struct list * record, struct sector_list * sector_list) {
    int status = 0;
    struct mercenary * mercenary;

    mercenary = pool_get(mercenary_tbl->pool);
    if(!mercenary) {
        status = panic("out of memory");
    } else {
        if(mercenary_create(mercenary, record, sector_list)) {
            status = panic("failed to create mercenary object");
        } else {
            if(list_push(&mercenary_tbl->list, mercenary)) {
                status = panic("failed to push list object");
            } else {
                if(map_insert(&mercenary_tbl->map_id, &mercenary->id, mercenary))
                    status = panic("failed to insert map object");
                if(status)
                    list_pop(&mercenary_tbl->list);
            }
            if(status)
                mercenary_destroy(mercenary);
        }
        if(status)
            pool_put(mercenary_tbl->pool, mercenary);
    }

    return status;
}

int constant_create(struct constant * constant, struct list * record, struct pool * range_node_pool, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    int toggle = 0;
    long min;
    long max;

    memset(constant, 0, sizeof(*constant));

    if(record->size < 3 || (record->size - 3) % 2 != 0) {
        status = panic("row is missing columns");
    } else if(range_create(&constant->range, range_node_pool)) {
        status = panic("failed to create range object");
    } else {
        field = 0;
        string = list_start(record);
        while(string && !status) {
            if(field < record->size - 3) {
                if(string_strtol(string, 10, toggle ? &min : &max)) {
                    status = panic("failed to strtol string object");
                } else if(toggle && range_add(&constant->range, min, max)) {
                    status = panic("failed to add range object");
                } else {
                    toggle = !toggle;
                }
            } else {
                switch(record->size - field) {
                    case 3: char_create(sector_list, string, &constant->name); break;
                    case 2: string_strtol(string, 10, &constant->value); break;
                    case 1: char_create(sector_list, string, &constant->macro); break;
                    default: status = panic("row has too many columns"); break;
                }
            }
            field++;
            string = list_next(record);
        }
    }

    if(status)
        constant_destroy(constant);

    return status;
}

void constant_destroy(struct constant * constant) {
    range_destroy(&constant->range);
    char_destroy(constant->name);
    char_destroy(constant->macro);
}

int constant_tbl_create(struct constant_tbl * constant_tbl, struct pool_map * pool_map) {
    int status = 0;

    constant_tbl->pool = pool_map_get(pool_map, sizeof(struct constant));
    if(!constant_tbl->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&constant_tbl->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&constant_tbl->map_macro, string_compare, pool_map_get(pool_map, sizeof(struct map_node))))
                status = panic("failed to create map object");
            if(status)
                list_destroy(&constant_tbl->list);
        }
    }

    return status;
}

void constant_tbl_destroy(struct constant_tbl * constant_tbl) {
    struct constant * constant;

    constant = list_pop(&constant_tbl->list);
    while(constant) {
        constant_destroy(constant);
        pool_put(constant_tbl->pool, constant);
        constant = list_pop(&constant_tbl->list);
    }

    map_destroy(&constant_tbl->map_vip_status);
    map_destroy(&constant_tbl->map_sizes);
    map_destroy(&constant_tbl->map_sc_start);
    map_destroy(&constant_tbl->map_sc_end);
    map_destroy(&constant_tbl->map_readparam);
    map_destroy(&constant_tbl->map_races);
    map_destroy(&constant_tbl->map_options);
    map_destroy(&constant_tbl->map_mapflags);
    map_destroy(&constant_tbl->map_locations);
    map_destroy(&constant_tbl->map_jobs);
    map_destroy(&constant_tbl->map_itemgroups);
    map_destroy(&constant_tbl->map_gettimes);
    map_destroy(&constant_tbl->map_elements);
    map_destroy(&constant_tbl->map_effects);
    map_destroy(&constant_tbl->map_classes);
    map_destroy(&constant_tbl->map_announces);
    map_destroy(&constant_tbl->map_macro);
    list_destroy(&constant_tbl->list);
}

int constant_tbl_json_create(struct constant_tbl * constant_tbl, struct json_node * node) {
    int status = 0;

    if( constant_tbl_add_map(constant_tbl, node, "announces", &constant_tbl->map_announces) ||
        constant_tbl_add_map(constant_tbl, node, "classes", &constant_tbl->map_classes) ||
        constant_tbl_add_map(constant_tbl, node, "effects", &constant_tbl->map_effects) ||
        constant_tbl_add_map(constant_tbl, node, "elements", &constant_tbl->map_elements) ||
        constant_tbl_add_map(constant_tbl, node, "gettimes", &constant_tbl->map_gettimes) ||
        constant_tbl_add_map(constant_tbl, node, "itemgroups", &constant_tbl->map_itemgroups) ||
        constant_tbl_add_map(constant_tbl, node, "jobs", &constant_tbl->map_jobs) ||
        constant_tbl_add_map(constant_tbl, node, "locations", &constant_tbl->map_locations) ||
        constant_tbl_add_map(constant_tbl, node, "mapflags", &constant_tbl->map_mapflags) ||
        constant_tbl_add_map(constant_tbl, node, "options", &constant_tbl->map_options) ||
        constant_tbl_add_map(constant_tbl, node, "races", &constant_tbl->map_races) ||
        constant_tbl_add_map(constant_tbl, node, "readparam", &constant_tbl->map_readparam) ||
        constant_tbl_add_map(constant_tbl, node, "sc_end", &constant_tbl->map_sc_end) ||
        constant_tbl_add_map(constant_tbl, node, "sc_start", &constant_tbl->map_sc_start) ||
        constant_tbl_add_map(constant_tbl, node, "sizes", &constant_tbl->map_sizes) ||
        constant_tbl_add_map(constant_tbl, node, "vip_status", &constant_tbl->map_vip_status) )
        status = panic("failed to add map constant table object");

    return status;
}

int constant_tbl_add(struct constant_tbl * constant_tbl, struct list * record, struct pool * range_node_pool, struct sector_list * sector_list) {
    int status = 0;
    struct constant * constant;

    constant = pool_get(constant_tbl->pool);
    if(!constant) {
        status = panic("out of memory");
    } else {
        if(constant_create(constant, record, range_node_pool, sector_list)) {
            status = panic("failed to create constant object");
        } else {
            if(list_push(&constant_tbl->list, constant)) {
                status = panic("failed to push list object");
            } else {
                if(map_insert(&constant_tbl->map_macro, constant->macro, constant))
                    status = panic("failed insert map object");
                if(status)
                    list_pop(&constant_tbl->list);
            }
            if(status)
                constant_destroy(constant);
        }
        if(status)
            pool_put(constant_tbl->pool, constant);
    }

    return status;
}

int constant_tbl_add_map(struct constant_tbl * constant_tbl, struct json_node * node, char * key, struct map * map) {
    int status = 0;
    struct json_node * array;
    struct json_node * element;
    char * string;
    struct constant * constant;

    array = json_object_get(node, key);
    if(!array) {
        status = panic("failed to get json object - %s", key);
    } else if(map_create(map, string_compare, constant_tbl->map_macro.pool)) {
        status = panic("failed to create map object");
    } else {
        element = json_array_start(array);
        while(element && !status) {
            string = json_string_get(element);
            if(!string) {
                status = panic("failed to get string object");
            } else {
                constant = map_search(&constant_tbl->map_macro, string);
                if(!constant) {
                    status = panic("failed to get constant object - %s", string);
                } else if(map_insert(map, constant->macro, constant)) {
                    status = panic("failed to insert map object");
                }
            }
            element = json_array_next(array);
        }
        if(status)
            map_destroy(map);
    }

    return status;
}

int db_item_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(item_tbl_add(&db->item_tbl, record, db->sector_list))
        status = panic("failed to add item table object");

    return status;
}

int db_item_tbl_create(struct db * db, struct csv * csv) {
    int status = 0;

    if(item_tbl_create(&db->item_tbl, db->pool_map)) {
        status = panic("failed to create item table object");
    } else {
        if(csv_parse(csv, "item_db.txt", db_item_tbl_create_cb, db))
            status = panic("failed to parse csv object");
        if(status)
            item_tbl_destroy(&db->item_tbl);
    }

    return status;
}

int db_item_combo_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(item_combo_tbl_add(&db->item_combo_tbl, record, db->sector_list))
        status = panic("failed to add item combo table object");

    return status;
}

int db_item_combo_tbl_create(struct db * db, struct csv * csv) {
    int status = 0;
    struct item_combo * item_combo;

    if(item_combo_tbl_create(&db->item_combo_tbl, db->pool_map)) {
        status = panic("failed to create item combo table object");
    } else {
        if(csv_parse(csv, "item_combo_db.txt", db_item_combo_tbl_create_cb, db)) {
            status = panic("failed to parse csv object");
        } else {
            item_combo = list_start(&db->item_combo_tbl.list);
            while(item_combo && !status) {
                if(item_tbl_add_combo(&db->item_tbl, item_combo))
                    status = panic("failed to add item combo to item table object");
                item_combo = list_next(&db->item_combo_tbl.list);
            }
        }
        if(status)
            item_combo_tbl_destroy(&db->item_combo_tbl);
    }

    return status;
}

int db_skill_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(skill_tbl_add(&db->skill_tbl, record, db->sector_list))
        status = panic("failed to add skill table object");

    return status;
}

int db_skill_tbl_create(struct db * db, struct csv * csv) {
    int status = 0;

    if(skill_tbl_create(&db->skill_tbl, db->pool_map)) {
        status = panic("failed to create skill table object");
    } else {
        if(csv_parse(csv, "skill_db.txt", db_skill_tbl_create_cb, db))
            status = panic("failed to parse csv object");
        if(status)
            skill_tbl_destroy(&db->skill_tbl);
    }

    return status;
}

int db_mob_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(mob_tbl_add(&db->mob_tbl, record, db->sector_list))
        status = panic("failed to add mob table object");

    return status;
}

int db_mob_tbl_create(struct db * db, struct csv * csv) {
    int status = 0;

    if(mob_tbl_create(&db->mob_tbl, db->pool_map)) {
        status = panic("failed to create mob table object");
    } else {
        if(csv_parse(csv, "mob_db.txt", db_mob_tbl_create_cb, db))
            status = panic("failed to parse csv object");
        if(status)
            mob_tbl_destroy(&db->mob_tbl);
    }

    return status;
}

int db_mob_race2_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(mob_race2_tbl_add(&db->mob_race2_tbl, record, db->sector_list))
        status = panic("failed to add mob race 2 table object");

    return status;
}

int db_mob_race2_tbl_create(struct db * db, struct csv * csv) {
    int status = 0;

    if(mob_race2_tbl_create(&db->mob_race2_tbl, db->pool_map)) {
        status = panic("failed to create mob race 2 table object");
    } else {
        if(csv_parse(csv, "mob_race2_db.txt", db_mob_race2_tbl_create_cb, db))
            status = panic("failed to parse csv object");
        if(status)
            mob_race2_tbl_destroy(&db->mob_race2_tbl);
    }

    return status;
}

int db_produce_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(produce_tbl_add(&db->produce_tbl, record, db->sector_list))
        status = panic("failed to add produce table object");

    return status;
}

int db_produce_tbl_create(struct db * db, struct csv * csv) {
    int status = 0;

    if(produce_tbl_create(&db->produce_tbl, db->pool_map)) {
        status = panic("failed to create produce table object");
    } else {
        if(csv_parse(csv, "produce_db.txt", db_produce_tbl_create_cb, db))
            status = panic("failed to parse csv object");
        if(status)
            produce_tbl_destroy(&db->produce_tbl);
    }

    return status;
}

int db_mercenary_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(mercenary_tbl_add(&db->mercenary_tbl, record, db->sector_list))
        status = panic("failed to add mercenary table object");

    return status;
}

int db_mercenary_tbl_create(struct db * db, struct csv * csv) {
    int status = 0;

    if(mercenary_tbl_create(&db->mercenary_tbl, db->pool_map)) {
        status = panic("failed to create mercenary table object");
    } else {
        if(csv_parse(csv, "mercenary_db.txt", db_mercenary_tbl_create_cb, db))
            status = panic("failed to parse csv object");
        if(status)
            mercenary_tbl_destroy(&db->mercenary_tbl);
    }

    return status;
}

int db_constant_tbl_create_cb(struct list * record, void * data) {
    int status = 0;
    struct db * db = data;

    if(constant_tbl_add(&db->constant_tbl, record, db->range_node_pool, db->sector_list))
        status = panic("failed to add constant table object");

    return status;
}

int db_constant_tbl_create(struct db * db, struct csv * csv, struct json * json) {
    int status = 0;

    if(constant_tbl_create(&db->constant_tbl, db->pool_map)) {
        status = panic("failed to create constant table object");
    } else {
        if(csv_parse(csv, "const_db.txt", db_constant_tbl_create_cb, db)) {
            status = panic("failed to parse csv object");
        } else if(json_parse(json, "constants.json")) {
            status = panic("failed to parse json object");
        } else {
            if(constant_tbl_json_create(&db->constant_tbl, json->root))
                status = panic("failed to create json constant table object");
            json_clear(json);
        }
        if(status)
            constant_tbl_destroy(&db->constant_tbl);
    }

    return status;
}

int db_create(struct db * db, struct pool_map * pool_map, struct sector_list * sector_list, struct csv * csv, struct json * json) {
    int status = 0;

    memset(db, 0, sizeof(*db));

    db->pool_map = pool_map;
    db->sector_list = sector_list;
    db->range_node_pool = pool_map_get(db->pool_map, sizeof(struct range_node));
    if(db_item_tbl_create(db, csv)) {
        status = panic("failed to create item table object");
    } else if(db_item_combo_tbl_create(db, csv)) {
        status = panic("failed to create item combo table object");
    } else if(db_skill_tbl_create(db, csv)) {
        status = panic("failed to create skill table object");
    } else if(db_mob_tbl_create(db, csv)) {
        status = panic("failed to create mob table object");
    } else if(db_mob_race2_tbl_create(db, csv)) {
        status = panic("failed to create mob race 2 table object");
    } else if(db_produce_tbl_create(db, csv)) {
        status = panic("failed to create produce table object");
    } else if(db_mercenary_tbl_create(db, csv)) {
        status = panic("failed to create mercenary table object");
    } else if(db_constant_tbl_create(db, csv, json)) {
        status = panic("failed to create constant table object");
    }

    if(status)
        db_destroy(db);

    return status;
}

void db_destroy(struct db * db) {
    constant_tbl_destroy(&db->constant_tbl);
    mercenary_tbl_destroy(&db->mercenary_tbl);
    produce_tbl_destroy(&db->produce_tbl);
    mob_race2_tbl_destroy(&db->mob_race2_tbl);
    mob_tbl_destroy(&db->mob_tbl);
    skill_tbl_destroy(&db->skill_tbl);
    item_combo_tbl_destroy(&db->item_combo_tbl);
    item_tbl_destroy(&db->item_tbl);
}
