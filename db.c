#include "db.h"

int data_create(struct schema *, enum type, int, struct pool *, struct data **);
void data_destroy(struct schema *, struct data *);

int parser_node(struct parser *, enum event_type, struct string *, struct data *);
int parser_event(enum event_type, struct string *, void *);

int data_create(struct schema * schema, enum type type, int mark, struct pool * pool, struct data ** result) {
    int status = 0;
    struct data * data;

    data = pool_get(schema->pool);
    if(!data) {
        status = panic("out of memory");
    } else {
        data->type = type;
        data->mark = mark;
        data->data = NULL;
        data->next = NULL;
        if(map_create(&data->map, (map_compare_cb) strcmp, pool))
            status = panic("failed to create map object");
        if(status) {
            pool_put(schema->pool, data);
        } else {
            *result = data;
        }
    }

    return status;
}

void data_destroy(struct schema * schema, struct data * data) {
    map_destroy(&data->map);
    pool_put(schema->pool, data);
}

int schema_create(struct schema * schema, struct heap * heap) {
    int status = 0;

    schema->pool = heap_pool(heap, sizeof(struct data));
    if(!schema->pool) {
        status = panic("failed to pool heap object");
    } else {
        if(data_create(schema, list, 0, heap->map_pool, &schema->root)) {
            status = panic("failed to create data object");
        } else {
            if(list_create(&schema->list, heap->list_pool)) {
                status = panic("failed to create list object");
            } else {
                if(list_push(&schema->list, schema->root))
                    status = panic("failed to push list object");
                if(status)
                    list_destroy(&schema->list);
            }
            if(status)
                data_destroy(schema, schema->root);
        }
    }

    return status;
}

void schema_destroy(struct schema * schema) {
    struct data * data;

    data = list_pop(&schema->list);
    while(data) {
        data_destroy(schema, data);
        data = list_pop(&schema->list);
    }
    list_destroy(&schema->list);
}

int schema_push(struct schema * schema, enum type type, int mark, char * key) {
    int status = 0;
    struct data * data;

    if(data_create(schema, type, mark, schema->root->map.pool, &data)) {
        status = panic("failed to create data object");
    } else {
        if(list_push(&schema->list, data)) {
            status = panic("failed to push list object");
        } else {
            if(schema->root->type == list) {
                if(schema->root->data) {
                    status = panic("invalid data");
                } else {
                    schema->root->data = data;
                }
            } else if(schema->root->type == map) {
                if(!key) {
                    status = panic("invalid key");
                } else if(map_insert(&schema->root->map, key, data)) {
                    status = panic("failed to insert map object");
                }
            }
            if(status) {
                /* skip push on error */
            } else if(data->type == list || data->type == map) {
                data->next = schema->root;
                schema->root = data;
            }
            if(status)
                list_pop(&schema->list);
        }
        if(status)
            data_destroy(schema, data);
    }

    return status;
}

void schema_pop(struct schema * schema) {
    if(schema->root->next)
        schema->root = schema->root->next;
}

struct data * schema_top(struct schema * schema) {
    struct data * data = NULL;

    if(schema->root->data) {
        data = schema->root->data;
        schema->root->data = NULL;
    }

    return data;
}

struct data * schema_load(struct schema * schema, struct markup * markup) {
    int status = 0;
    struct markup * root = NULL;

    while(markup->level) {
        while(root && root->level >= markup->level) {
            schema_pop(schema);
            root = root->next;
        }
        if(schema_push(schema, markup->type, markup->mark, markup->key)) {
            status = panic("failed to push schema object");
        } else if(markup->type == list || markup->type == map) {
            markup->next = root;
            root = markup;
        }
        markup++;
    }

    while(root) {
        schema_pop(schema);
        root = root->next;
    }

    return status ? NULL : schema_top(schema);
}

int parser_create(struct parser * parser, size_t size, struct heap * heap) {
    int status = 0;

    parser->size = size;

    if(csv_create(&parser->csv, parser->size, heap)) {
        status = panic("failed to create csv object");
    } else {
        if(json_create(&parser->json, parser->size)) {
            status = panic("failed to create json object");
        } else {
            if(yaml_create(&parser->yaml, parser->size, heap))
                status = panic("failed to create yaml object");
            if(status)
                json_destroy(&parser->json);
        }
        if(status)
            csv_destroy(&parser->csv);
    }

    return status;
}

void parser_destroy(struct parser * parser) {
    yaml_destroy(&parser->yaml);
    json_destroy(&parser->json);
    csv_destroy(&parser->csv);
}

int parser_node(struct parser * parser, enum event_type event, struct string * string, struct data * data) {
    int status = 0;

    if(data->type == list) {
        if(event != list_begin) {
            status = panic("invalid event");
        } else {
            if(parser->callback(start, data->mark, NULL, parser->context)) {
                status = panic("failed to process start event");
            } else {
                data->next = parser->root;
                parser->root = data;
            }
        }
    } else if(data->type == map) {
        if(event != map_begin) {
            status = panic("invalid event");
        } else {
            if(parser->callback(start, data->mark, NULL, parser->context)) {
                status = panic("failed to process start event");
            } else {
                data->next = parser->root;
                parser->root = data;
            }
        }
    } else {
        if(event != scalar) {
            status = panic("invalid event");
        } else if(parser->callback(next, data->mark, string, parser->context)) {
            status = panic("failed to process next event");
        }
    }

    return status;
}

int parser_event(enum event_type event, struct string * string, void * context) {
    int status = 0;
    struct parser * parser = context;

    if(parser->data) {
        if(parser_node(parser, event, string, parser->data)) {
            status = panic("failed to node parser object");
        } else {
            parser->data = NULL;
        }
    } else {
        if(!parser->root) {
            status = panic("invalid data");
        } else {
            if(parser->root->type == list) {
                if(event == list_end) {
                    if(parser->callback(end, parser->root->mark, NULL, parser->context)) {
                        status = panic("failed to process end event");
                    } else {
                        parser->root = parser->root->next;
                    }
                } else if(parser_node(parser, event, string, parser->root->data)) {
                    status = panic("failed to node parser object");
                }
            } else if(parser->root->type == map) {
                if(event == map_end) {
                    if(parser->callback(end, parser->root->mark, NULL, parser->context)) {
                        status = panic("failed to process end event");
                    } else {
                        parser->root = parser->root->next;
                    }
                } else if(event == scalar) {
                    parser->data = map_search(&parser->root->map, string->string);
                    if(!parser->data)
                        status = panic("invalid key - %s", string->string);
                } else {
                    status = panic("invalid event");
                }
            }
        }
    }

    return status;
}

int parser_parse(struct parser * parser, const char * path, struct data * data, parser_cb callback, void * context) {
    int status = 0;
    char * ext;

    parser->root = NULL;
    parser->data = data;
    parser->callback = callback;
    parser->context = context;

    ext = strrchr(path, '.');
    if(!ext) {
        status = panic("failed to get file extension - %s", path);
    } else {
        if(!strcmp(ext, ".txt")) {
            if(csv_parse(&parser->csv, path, parser->size, parser_event, parser))
                status = panic("failed to parse csv object");
        } else if(!strcmp(ext, ".json")) {
            if(json_parse(&parser->json, path, parser->size, parser_event, parser))
                status = panic("failed to parse json object");
        } else if(!strcmp(ext, ".yaml") || !strcmp(ext, ".yml")) {
            if(yaml_parse(&parser->yaml, path, parser->size, parser_event, parser))
                status = panic("failed to parse yaml object");
        }

        if(parser->root)
            status = panic("failed to parse %s", path);
    }

    return status;
}
