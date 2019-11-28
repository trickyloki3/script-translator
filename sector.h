#ifndef sector_h
#define sector_h

#include "range.h"

struct sector {
    char * buffer;
    struct pool pool;
    struct range range;
};

int sector_create(struct sector *, long);
void sector_destroy(struct sector *);
void * sector_malloc(struct sector *, long);
void sector_free(void *);
long sector_size(void *);
void sector_print(struct sector *);

#endif
