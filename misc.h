#ifndef misc_h
#define misc_h

#include "parser.h"

struct long_array {
    long * array;
    size_t count;
};

int long_compare(void *, void *);

int string_store(struct string *,struct store *,  struct string **);
int string_strtod(struct string *, double *);
int string_strtol(struct string *, int, long *);
int string_strtoul(struct string *, int, unsigned long *);
int string_strtol_split(struct string *, int, char, struct store *, struct long_array **);
int string_strtol_splitv(struct string *, int, int, ...);

#endif
