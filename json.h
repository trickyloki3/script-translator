#ifndef json_h
#define json_h

#include "event.h"

struct json {
    void * parser;
    event_cb callback;
    void * context;
};

int json_parse(const char *, event_cb, void *);

#endif
