#ifndef yaml_h
#define yaml_h

#include "pool_map.h"
#include "sector_list.h"
#include "sector_string.h"

struct yaml {
    int indent;
};

int yaml_create(struct yaml *, struct pool_map *, struct sector_list *);
void yaml_destroy(struct yaml *);
int yaml_parse(struct yaml *, const char *);

#endif
