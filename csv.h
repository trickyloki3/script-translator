#ifndef csv_h
#define csv_h

#include "array.h"
#include "list.h"

typedef int (* csv_process_cb) (struct list *, void *);

struct csv {
    struct list string;
    struct list record;
    csv_process_cb process;
    void * data;
};

int csv_create(struct csv *, struct pool *);
void csv_destroy(struct csv *);
int csv_parse(struct csv *, const char *, csv_process_cb, void *);
struct string * csv_get_string(struct csv *);
int csv_put_string(struct csv *, struct string *);
int csv_push_field_string(struct csv *, struct string *);
int csv_push_field_strdup(struct csv *, char *, size_t);
int csv_push_field_empty(struct csv *);
int csv_process_record(struct csv *);
int csv_clear_record(struct csv *);

#endif
