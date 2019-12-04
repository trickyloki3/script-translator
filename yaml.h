#ifndef yaml_h
#define yaml_h

#include "heap.h"

struct yaml {

};

int yaml_create(struct yaml *, struct heap *);
void yaml_destroy(struct yaml *);
int yaml_parse(struct yaml *, const char *);

#endif
