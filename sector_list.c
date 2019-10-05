#include "sector_list.h"

int sector_list_sector_create(struct sector_list *, struct sector **);

int sector_list_create(struct sector_list * sector_list, size_t size, struct pool * sector_node_pool, struct pool * list_node_pool) {
    int status = 0;

    sector_list->size = size;
    sector_list->pool = sector_node_pool;
    if(list_create(&sector_list->list, list_node_pool))
        status = panic("failed to create list object");

    return status;
}

void sector_list_destroy(struct sector_list * sector_list) {
    struct sector * sector;

    sector = list_pop(&sector_list->list);
    while(sector) {
        sector_destroy(sector);
        free(sector);
        sector = list_pop(&sector_list->list);
    }

    list_destroy(&sector_list->list);
}

int sector_list_sector_create(struct sector_list * sector_list, struct sector ** result) {
    int status = 0;
    struct sector * sector;

    sector = malloc(sizeof(*sector));
    if(!sector) {
        status = panic("out of memory");
    } else {
        if(sector_create(sector, sector_list->size, sector_list->pool)) {
            status = panic("failed to create sector object");
        } else {
            if(list_push(&sector_list->list, sector)) {
                status = panic("failed to push list object");
            } else {
                *result = sector;
            }
            if(status)
                sector_destroy(sector);
        }
        if(status)
            free(sector);
    }

    return status;
}

void * sector_list_malloc(struct sector_list * sector_list, size_t size) {
    void * object = NULL;
    struct sector * sector;

    if(size <= sector_list->size) {
        sector = list_start(&sector_list->list);
        while(sector && !object) {
            object = sector_malloc(sector, size);
            sector = list_next(&sector_list->list);
        }

        if(!object && !sector_list_sector_create(sector_list, &sector))
            object = sector_malloc(sector, size);
    }

    return object;
}

void sector_list_free(void * object) {
    sector_free(object);
}
