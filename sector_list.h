#ifndef sector_list_h
#define sector_list_h

#include "list.h"
#include "sector.h"

struct sector_list {
    size_t size;
    struct pool * pool;
    struct list list;
};

int sector_list_create(struct sector_list *, size_t, struct pool *, struct pool *);
void sector_list_destroy(struct sector_list *);
void * sector_list_malloc(struct sector_list *, size_t);
void sector_list_free(void *);

#endif
