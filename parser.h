#ifndef parser_h
#define parser_h

#include "yaml_reader.h"

struct parser {
    struct yaml yaml;
    struct meta meta;
};

int parser_create(struct parser *, size_t);
void parser_destroy(struct parser *);
int parser_file(struct parser *, struct tag *, const char *, yaml_reader_cb, void *);

#endif
