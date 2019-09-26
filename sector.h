#ifndef sector_h
#define sector_h

#include "pool.h"

struct sector_node {
    size_t min;
    size_t max;
    struct sector_node * next;
    struct sector_node * prev;
};

struct sector {
    char * buffer;
    struct pool * pool;
    struct sector_node * root;
};

int sector_create(struct sector *, size_t, struct pool *);
void sector_destroy(struct sector *);
void * sector_malloc(struct sector *, size_t);
void sector_free(void *);
void sector_print(struct sector *);

#endif
