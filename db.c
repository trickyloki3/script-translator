#include "db.h"

int long_compare(void *, void *);
int string_compare(void *, void *);

int char_create(struct sector_list *, struct string *, char **);
void char_destroy(char *);

int item_create(struct item *, struct list *, struct sector_list *);
void item_destroy(struct item *);

int item_tbl_create(struct item_tbl *, struct pool_map *);
void item_tbl_destroy(struct item_tbl *);
int item_tbl_add(struct item_tbl *, struct list *, struct sector_list *);

int db_item_tbl_create_cb(struct list *, void *);
int db_item_tbl_create(struct db *, struct csv *);

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

int item_create(struct item * item, struct list * record, struct sector_list * sector_list) {
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

    if(!status && field != 22)
        status = panic("row is missing columns");

    if(status)
        item_destroy(item);

    return status;
}

void item_destroy(struct item * item) {
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
        if(item_create(item, record, sector_list)) {
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

int db_create(struct db * db, struct pool_map * pool_map, struct sector_list * sector_list, struct csv * csv) {
    int status = 0;

    db->pool_map = pool_map;
    db->sector_list = sector_list;
    if(db_item_tbl_create(db, csv))
        status = panic("failed to create item table object");

    return status;
}

void db_destroy(struct db * db) {
    item_tbl_destroy(&db->item_tbl);
}
