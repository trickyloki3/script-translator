#ifndef csv_h
#define csv_h

#include "event.h"

struct csv {
    int index;
    parser_cb callback;
    void * context;
};

int csv_parse(const char *, parser_cb, void *);

#endif
