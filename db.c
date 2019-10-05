#include "db.h"

int long_compare(void *, void *);
int string_compare(void *, void *);

int char_create(struct sector_list *, struct string *, char **);
void char_destroy(char *);

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

int db_item_tbl_create_cb(struct list *, void *);
int db_item_tbl_create(struct db *, struct csv *);
int db_item_combo_tbl_create_cb(struct list *, void *);
int db_item_combo_tbl_create(struct db *, struct csv *);
int db_skill_tbl_create_cb(struct list *, void *);
int db_skill_tbl_create(struct db *, struct csv *);

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int string_compare(void * x, void * y) {
    return strcmp(x, y);
}

int char_create(struct sector_list * sector_list, struct string * string, char ** result) {
    int status = 0;
    char * object;

    object = sector_list_malloc(sector_list, string->offset + 1);
    if(!object) {
        status = panic("out of memory");
    } else {
        *result = memcpy(object, string->string, string->offset + 1);
    }

    return status;
}

void char_destroy(char * object) {
    sector_list_free(object);
}

int item_create(struct item * item, struct list * record, struct pool * list_node_pool, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(item, 0, sizeof(*item));

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

    if(!status && field != 22) {
        status = panic("row is missing columns");
    } else if(list_create(&item->combo, list_node_pool)) {
        status = panic("failed to create list object");
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
        status = panic("out of memory");
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

    if(!status && field != 2)
        status = panic("row is missing columns");

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

    if(!status && field != 18)
        status = panic("row is missing columns");

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
            status = panic("failed to skill object");
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

int db_create(struct db * db, struct pool_map * pool_map, struct sector_list * sector_list, struct csv * csv) {
    int status = 0;

    db->pool_map = pool_map;
    db->sector_list = sector_list;
    if(db_item_tbl_create(db, csv)) {
        status = panic("failed to create item table object");
    } else if(db_item_combo_tbl_create(db, csv)) {
        status = panic("failed to create item combo table object");
    } else if(db_skill_tbl_create(db, csv)) {
        status = panic("failed to create skill table object");
    }

    return status;
}

void db_destroy(struct db * db) {
    skill_tbl_destroy(&db->skill_tbl);
    item_combo_tbl_destroy(&db->item_combo_tbl);
    item_tbl_destroy(&db->item_tbl);
}
