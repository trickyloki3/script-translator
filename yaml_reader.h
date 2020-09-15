#ifndef yaml_reader_h
#define yaml_reader_h

#include "yaml.h"
#include "meta.h"

typedef int (* yaml_reader_cb) (enum yaml_event, int, char *, size_t, void *);

int yaml_reader_parse(struct yaml *, const char *, struct meta *, yaml_reader_cb, void *);

#endif
