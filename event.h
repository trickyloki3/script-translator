#ifndef event_h
#define event_h

#include "array.h"

enum event_type {
    event_list_start,
    event_list_end,
    event_map_start,
    event_map_end,
    event_scalar
};

typedef int (* event_cb) (enum event_type, struct string *, void *);

#endif
