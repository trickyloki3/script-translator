#include "db.h"

int long_compare(void *, void *);
int string_compare(void *, void *);

int char_create(struct sector_list *, struct string *, char **);
void char_destroy(char *);

int item_create(struct item *, struct list *, struct pool *, struct sector_list *);
void item_destroy(struct item *);
int item_tbl_process(struct list *, void *);

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

int item_create(struct item * item, struct list * record, struct pool * pool, struct sector_list * sector_list) {
    int status = 0;
    size_t field;
    struct string * string;

    memset(item, 0, sizeof(*item));

    field = 0;
    string = list_poll(record);
    while(string && !status) {
        switch(field) {
            case 0: status = string_strtol(string, 10, &item->id); break;
            case 1: status = char_create(sector_list, string, &item->aegis); break;
            case 2: status = char_create(sector_list, string, &item->name); break;
            case 3: status = string_strtol(string, 10, &item->type); break;
            case 4: status = string_strtol(string, 10, &item->buy); break;
            case 5: status = string_strtol(string, 10, &item->sell); break;
            case 6: status = string_strtol(string, 10, &item->weight); break;
            case 7: status = string_strtol_splitv(string, 10, ':', &item->atk, &item->matk, NULL); break;
            case 8: status = string_strtol(string, 10, &item->def); break;
            case 9: status = string_strtol(string, 10, &item->range); break;
            case 10: status = string_strtol(string, 10, &item->slots); break;
            case 11: status = string_strtoul(string, 16, &item->job); break;
            case 12: status = string_strtoul(string, 10, &item->upper); break;
            case 13: status = string_strtol(string, 10, &item->gender); break;
            case 14: status = string_strtoul(string, 10, &item->location); break;
            case 15: status = string_strtol(string, 10, &item->weapon_level); break;
            case 16: status = string_strtol_splitv(string, 10, ':', &item->base_level, &item->max_level, NULL); break;
            case 17: status = string_strtol(string, 10, &item->refineable); break;
            case 18: status = string_strtol(string, 10, &item->view); break;
            case 19: status = char_create(sector_list, string, &item->bonus); break;
            case 20: status = char_create(sector_list, string, &item->onequip); break;
            case 21: status = char_create(sector_list, string, &item->onunequip); break;
            default: status = panic("row has too many columns"); break;
        }
        field++;
        string = list_poll(record);
    }

    if(!status && field != 22){
        status = panic("row is missing columns");
    } else if(list_create(&item->combo, pool)) {
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

int item_tbl_process(struct list * record, void * data) {
    int status = 0;
    struct item_tbl * item_tbl = data;
    struct item * item;

    item = pool_get(item_tbl->item_pool);
    if(!item) {
        status = panic("out of memory");
    } else {
        if(item_create(item, record, item_tbl->list_node_pool, item_tbl->sector_list)) {
            status = panic("failed to process item object");
        } else {
            if(map_insert(&item_tbl->map_by_id, &item->id, item)) {
                status = panic("failed to insert map object");
            } else if(map_insert(&item_tbl->map_by_name, item->name, item)) {
                status = panic("failed to insert map object");
            }
            if(status)
                item_destroy(item);
        }

        if(status) {
            pool_put(item_tbl->item_pool, item);
        } else {
            item->next = item_tbl->root;
            item_tbl->root = item;
        }
    }

    return status;
}

int item_tbl_create(struct item_tbl * item_tbl, struct csv * csv, struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;
    struct pool * pool;

    if(pool_map_get(pool_map, sizeof(struct list_node), &item_tbl->list_node_pool)) {
        status = panic("failed to get pool map object");
    } else if(pool_map_get(pool_map, sizeof(struct item), &item_tbl->item_pool)) {
        status = panic("failed to get pool map object");
    } else {
        item_tbl->root = NULL;
        if(pool_map_get(pool_map, sizeof(struct map_node), &pool)) {
            status = panic("failed to get pool map object");
        } else {
            if(map_create(&item_tbl->map_by_id, long_compare, pool)) {
                status = panic("failed to create map object");
            } else {
                if(map_create(&item_tbl->map_by_name, string_compare, pool)) {
                    status = panic("failed to create map object");
                } else {
                    item_tbl->sector_list = sector_list;
                    if(csv_parse(csv, "item_db.txt", item_tbl_process, item_tbl))
                        status = panic("failed to parse csv object");
                    if(status)
                        map_destroy(&item_tbl->map_by_name);
                }
                if(status)
                    map_destroy(&item_tbl->map_by_id);
            }
        }
    }

    return status;
}

void item_tbl_destroy(struct item_tbl * item_tbl) {
    struct item * item;
    struct item * temp;

    item = item_tbl->root;
    while(item) {
        temp = item;
        item = item->next;
        item_destroy(temp);
        pool_put(item_tbl->item_pool, temp);
    }
    map_destroy(&item_tbl->map_by_name);
    map_destroy(&item_tbl->map_by_id);
}

int db_create(struct db * db, struct csv * csv, struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;

    if(item_tbl_create(&db->item_tbl, csv, pool_map, sector_list)) {
        status = panic("failed to create item table object");
    } else {
        if(status)
            item_tbl_destroy(&db->item_tbl);
    }

    return status;
}

void db_destroy(struct db * db) {
    item_tbl_destroy(&db->item_tbl);
}
