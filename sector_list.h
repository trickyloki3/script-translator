#ifndef sector_list_h
#define sector_list_h

#include "pool.h"
#include "list.h"
#include "sector.h"
#include "pool_map.h"

struct sector_list {
    struct pool * sector_pool;
    struct pool * sector_node_pool;
    size_t size;
    struct list list;
};

int sector_list_create(struct sector_list *, size_t, size_t, struct pool_map *);
void sector_list_destroy(struct sector_list *);
void * sector_list_malloc(struct sector_list *, size_t);
void sector_list_free(void *);

#endif
