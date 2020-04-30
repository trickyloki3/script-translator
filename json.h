#ifndef json_h
#define json_h

#include "event.h"

struct json {
    struct strbuf strbuf;
    int token;
    struct string * string;
    event_cb callback;
    void * context;
};

int json_create(struct json *, size_t);
void json_destroy(struct json *);
int json_parse(struct json *, const char *, event_cb, void *);
int json_token(struct json *, int, char *, size_t);

#endif
