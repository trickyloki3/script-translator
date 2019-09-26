#ifndef csv_h
#define csv_h

#include "pool.h"
#include "list.h"
#include "array.h"

struct csv {
    struct pool pool;
    struct list string;
    struct list record;
};

int csv_create(struct csv *, struct pool *);
void csv_destroy(struct csv *);
int csv_parse(struct csv *, const char *, size_t);
struct string * csv_get_string(struct csv *);
int csv_put_string(struct csv *, struct string *);
struct string * csv_strdup_string(struct csv *, char *, size_t);
int csv_push_field(struct csv *, struct string *);
int csv_push_field_empty(struct csv *);
int csv_clear_record(struct csv *);

#endif
