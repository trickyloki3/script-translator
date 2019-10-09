#ifndef aux_h
#define aux_h

#include "sector_list.h"

int long_compare(void *, void *);
int string_compare(void *, void *);

int char_create(struct sector_list *, struct string *, char **);
void char_destroy(char *);

#endif
