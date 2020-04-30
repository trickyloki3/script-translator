#ifndef event_h
#define event_h

#include "array.h"

enum event_type {
    event_list_start = 1,
    event_list_end,
    event_map_start,
    event_map_end,
    event_scalar
};

typedef int (* event_cb) (enum event_type, struct string *, void *);

enum parser_event {
    start,
    next,
    end
};

typedef int (* parser_cb) (enum parser_event, int, struct string *, void *);

#endif
