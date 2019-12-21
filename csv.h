#ifndef csv_h
#define csv_h

#include "event.h"
#include "heap.h"

struct csv {
    struct strbuf strbuf;
    struct list list;
    event_cb callback;
    void * context;
};

int csv_create(struct csv *, size_t, struct heap *);
void csv_destroy(struct csv *);
int csv_parse(struct csv *, const char *, size_t, event_cb, void *);
int csv_push(struct csv *);
int csv_pop(struct csv *);
void csv_reset(struct csv *);

#endif
