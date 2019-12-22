#ifndef event_h
#define event_h

#include "array.h"

enum event_type {
    list_begin,
    list_end,
    map_begin,
    map_end,
    scalar
};

typedef int (* event_cb) (enum event_type, struct string *, void *);

#endif
