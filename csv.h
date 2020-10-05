#ifndef csv_h
#define csv_h

#include "array.h"

enum csv_event {
    csv_start,
    csv_next,
    csv_end
};

typedef int (* csv_cb) (enum csv_event, int, struct string *, void *);

struct csv {
    int index;
    csv_cb cb;
    void * arg;
};

int csv_parse(const char *, csv_cb, void *);

#endif
