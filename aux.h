#ifndef aux_h
#define aux_h

#include "sector_list.h"

int long_compare(void *, void *);
int string_compare(void *, void *);

int char_create(struct sector_list *, struct string *, char **);
int char_create2(struct sector_list *, char *, size_t, char **);
size_t char_size(char *);
void char_destroy(char *);

#endif
