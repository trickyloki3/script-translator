#include "db.h"

int long_compare(void *, void *);
int string_compare(void *, void *);

int item_create(struct item *, struct list *);
void item_destroy(struct item *);

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int string_compare(void * x, void * y) {
    return strcmp(x, y);
}

int item_create(struct item * item, struct list * record) {
    int status = 0;
    size_t field = 0;
    struct string * string;
    long list[2];

    string = list_poll(record);
    while(string && !status) {
        switch(field) {
            case 0: status = string_strtol(string, 10, &item->id); break;
            case 1: status = string_copy(&item->aegis, string); break;
            case 2: status = string_copy(&item->name, string); break;
            case 3: status = string_strtol(string, 10, &item->type); break;
            case 4: status = string_strtol(string, 10, &item->buy); break;
            case 5: status = string_strtol(string, 10, &item->sell); break;
            case 6: status = string_strtol(string, 10, &item->weight); break;
            case 7:
                memset(list, 0, sizeof(list));
                if(string_strtol_split(string, 10, ':', list, 2)) {
                    status = panic("failed to strtol split string object");
                } else {
                    item->atk = list[0];
                    item->matk = list[1];
                }
                break;
            break;
            case 8: status = string_strtol(string, 10, &item->def); break;
            case 9: status = string_strtol(string, 10, &item->range); break;
            case 10: status = string_strtol(string, 10, &item->slots); break;
            case 11: status = string_strtoul(string, 16, &item->job); break;
            case 12: status = string_strtoul(string, 10, &item->upper); break;
            case 13: status = string_strtol(string, 10, &item->gender); break;
            case 14: status = string_strtoul(string, 10, &item->location); break;
            case 15: status = string_strtol(string, 10, &item->weapon_level); break;
            case 16:
                memset(list, 0, sizeof(list));
                if(string_strtol_split(string, 10, ':', list, 2)) {
                    status = panic("failed to strtol split string object");
                } else {
                    item->base_level = list[0];
                    item->max_level = list[1];
                }
                break;
            case 17: status = string_strtol(string, 10, &item->refineable); break;
            case 18: status = string_strtol(string, 10, &item->view); break;
            case 19: status = string_copy(&item->bonus, string); break;
            case 20: status = string_copy(&item->onequip, string); break;
            case 21: status = string_copy(&item->onunequip, string); break;
            default: status = panic("row has too many columns"); break;
        }
        field++;
        string = list_poll(record);
    }

    if(!status && field != 22)
        status = panic("row is missing columns");

    return status;
}

void item_destroy(struct item * item) {
    string_destroy(&item->onunequip);
    string_destroy(&item->onequip);
    string_destroy(&item->bonus);
    string_destroy(&item->name);
    string_destroy(&item->aegis);
}

int item_tbl_process(struct list * record, void * data) {
    int status = 0;
    struct item_tbl * item_tbl = data;
    struct item * item;

    item = pool_get(item_tbl->pool);
    if(!item) {
        status = panic("out of memory");
    } else {
        memset(item, 0, sizeof(struct item));
        if(item_create(item, record)) {
            status = panic("failed to process item object");
        } else if(map_insert(&item_tbl->map_by_id, &item->id, item)) {
            status = panic("failed to insert map object");
        } else if(map_insert(&item_tbl->map_by_name, item->name.string, item)) {
            status = panic("failed to insert map object");
        }

        if(status) {
            item_destroy(item);
            pool_put(item_tbl->pool, item);
        } else {
            item->next = item_tbl->root;
            item_tbl->root = item;
        }
    }

    return status;
}

int item_tbl_create(struct item_tbl * item_tbl, struct csv * csv, struct pool_map * pool_map) {
    int status = 0;
    struct pool * pool;

    item_tbl->root = NULL;
    if(pool_map_get(pool_map, sizeof(struct item), 512, &item_tbl->pool)) {
        status = panic("failed to get pool map object");
    } else {
        if(pool_map_get(pool_map, sizeof(struct map_node), 512, &pool)) {
            status = panic("failed to get pool map object");
        } else {
            if(map_create(&item_tbl->map_by_id, long_compare, pool)) {
                status = panic("failed to create map object");
            } else {
                if(map_create(&item_tbl->map_by_name, string_compare, pool)) {
                    status = panic("failed to create map object");
                } else {
                    if(csv_parse(csv, "item_db.txt", 4096, item_tbl_process, item_tbl))
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
        pool_put(item_tbl->pool, temp);
    }
    map_destroy(&item_tbl->map_by_name);
    map_destroy(&item_tbl->map_by_id);
}

int db_create(struct db * db, struct csv * csv, struct pool_map * pool_map) {
    int status = 0;

    if(item_tbl_create(&db->item_tbl, csv, pool_map)) {
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
