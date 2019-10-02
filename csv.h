#ifndef csv_h
#define csv_h

#include "pool.h"
#include "list.h"
#include "pool_map.h"
#include "array.h"

typedef int (* csv_process_cb) (struct list *, void *);

struct csv {
    struct pool * string_pool;
    struct list string;
    struct list active;
    struct list record;
    size_t buffer_size;
    csv_process_cb process;
    void * data;
};

int csv_create(struct csv *, size_t, struct pool_map *);
void csv_destroy(struct csv *);
int csv_parse(struct csv *, const char *, csv_process_cb, void *);
struct string * csv_get_string(struct csv *);
int csv_put_string(struct csv *, struct string *);
struct string * csv_strdup_string(struct csv *, char *, size_t);
int csv_push_field(struct csv *, struct string *);
int csv_push_field_empty(struct csv *);
int csv_process_record(struct csv *);
int csv_clear_record(struct csv *);

#endif
