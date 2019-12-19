#ifndef event_h
#define event_h

#include "array.h"

enum event_type {
    event_list_start  = 1024,
    event_list_end,  // 1025
    event_map_start, // 1026
    event_map_end,   // 1027
    event_string     // 1028
};

typedef int (* event_cb) (enum event_type, struct string *, void *);

#endif
