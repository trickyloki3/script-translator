#ifndef db_h
#define db_h

#include "csv.h"
#include "json.h"
#include "yaml.h"

struct parser {
    size_t size;
    struct csv csv;
    struct json json;
    struct yaml yaml;
};

int parser_create(struct parser *, size_t, struct heap *);
void parser_destroy(struct parser *);
int parser_parse(struct parser *, const char *);

#endif
