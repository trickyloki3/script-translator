#include "parser.h"

static inline struct schema_node * schema_node_create(struct schema *, enum schema_type, int, struct pool *);
static inline void schema_node_destroy(struct schema *, struct schema_node *);
void schema_node_print(struct schema_node *, int, char *);

int parser_start(struct parser *, struct schema_node *, enum event_type, struct string *);
int parser_event(enum event_type, struct string *, void *);

static inline struct schema_node * schema_node_create(struct schema * schema, enum schema_type type, int mark, struct pool * pool) {
    int status = 0;
    struct schema_node * node;

    node = pool_get(schema->pool);
    if(node) {
        node->type = type;
        node->state = 0;
        node->mark = mark;
        node->list = NULL;
        node->next = NULL;
        if(map_create(&node->map, (map_compare_cb) strcmp, pool))
            status = panic("failed to create map object");
        if(status)
            pool_put(schema->pool, node);
    }

    return status ? NULL : node;
}

static inline void schema_node_destroy(struct schema * schema, struct schema_node * node) {
    struct map_kv kv;

    kv = map_start(&node->map);
    while(kv.key) {
        schema_node_destroy(schema, kv.value);
        kv = map_next(&node->map);
    }

    if(node->list)
        schema_node_destroy(schema, node->list);

    map_destroy(&node->map);
    pool_put(schema->pool, node);
}

void schema_node_print(struct schema_node * node, int indent, char * key) {
    int i;
    struct map_kv kv;

    for(i = 0; i < indent; i++)
        fputs("    ", stdout);

    if(key)
        fprintf(stdout, "[%s]", key);

    switch(node->type) {
        case list | map | string:
            fprintf(stdout, "[list, map, string][%d]\n", node->mark);
            break;
        case list | map:
            fprintf(stdout, "[list, map][%d]\n", node->mark);
            break;
        case list | string:
            fprintf(stdout, "[list, string][%d]\n", node->mark);
            break;
        case map | string:
            fprintf(stdout, "[map, string][%d]\n", node->mark);
            break;
        case list:
            fprintf(stdout, "[list][%d]\n", node->mark);
            break;
        case map:
            fprintf(stdout, "[map][%d]\n", node->mark);
            break;
        case string:
            fprintf(stdout, "[string][%d]\n", node->mark);
            break;
    }

    if(node->type & list)
        schema_node_print(node->list, indent + 1, NULL);

    if(node->type & map) {
        kv = map_start(&node->map);
        while(kv.key) {
            schema_node_print(kv.value, indent + 1, kv.key);
            kv = map_next(&node->map);
        }
    }
}

int schema_create(struct schema * schema, struct heap * heap) {
    int status = 0;

    schema->pool = heap_pool(heap, sizeof(struct schema_node));
    if(!schema->pool) {
        status = panic("failed to pool heap object");
    } else {
        schema->root = schema_node_create(schema, list, 0, heap->map_pool);
        if(!schema->root) {
            status = panic("failed to create schema node object");
        } else {
            schema->root->state = list;
        }
    }

    return status;
}

void schema_destroy(struct schema * schema) {
    schema_node_destroy(schema, schema->root);
}

void schema_clear(struct schema * schema) {
    if(schema->root->list) {
        schema_node_destroy(schema, schema->root->list);
        schema->root->list = NULL;
    }
}

int schema_push(struct schema * schema, enum schema_type type, int mark, char * key) {
    int status = 0;
    struct schema_node * node;

    node = schema_node_create(schema, type, mark, schema->root->map.pool);
    if(!node) {
        status = panic("failed to create schema node object");
    } else {
        if(key) {
            if(schema->root->type & map) {
                if(map_insert(&schema->root->map, key, node))
                    status = panic("failed to insert map object");
            } else {
                status = panic("expected map");
            }
        } else {
            if(schema->root->type & list) {
                if(schema->root->list) {
                    status = panic("invalid data");
                } else {
                    schema->root->list = node;
                }
            } else {
                status = panic("expected list");
            }
        }

        if(status) {
            schema_node_destroy(schema, node);
        } else if(node->type & list || node->type & map) {
            node->next = schema->root;
            schema->root = node;
        }
    }

    return status;
}

void schema_pop(struct schema * schema) {
    if(schema->root->next)
        schema->root = schema->root->next;
}

struct schema_node * schema_top(struct schema * schema) {
    return schema->root;
}

int schema_load(struct schema * schema, struct schema_markup * markup) {
    int status = 0;
    struct schema_markup * root = NULL;

    schema_clear(schema);

    while(markup->level && !status) {
        while(root && root->level >= markup->level) {
            schema_pop(schema);
            root = root->next;
        }
        if(schema_push(schema, markup->type, markup->mark, markup->key)) {
            status = panic("failed to push schema object");
        } else if(markup->type & list || markup->type & map) {
            markup->next = root;
            root = markup;
        }
        markup++;
    }

    while(root) {
        schema_pop(schema);
        root = root->next;
    }

    return status;
}

void schema_print(struct schema * schema) {
    struct schema_node * node;

    node = schema->root->list;
    if(node)
        schema_node_print(node, 0, NULL);
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

int parser_start(struct parser * parser, struct schema_node * node, enum event_type type, struct string * value) {
    int status = 0;

    if(type == event_list_start) {
        if(node->type & list) {
            if(parser->callback(start, node->mark, NULL, parser->context)) {
                status = panic("failed to process start event");
            } else {
                node->state = list;
                node->next = parser->root;
                parser->root = node;
            }
        } else {
            status = panic("unexpected list");
        }
    } else if(type == event_map_start) {
        if(node->type & map) {
            if(parser->callback(start, node->mark, NULL, parser->context)) {
                status = panic("failed to process start event");
            } else {
                node->state = map;
                node->next = parser->root;
                parser->root = node;
            }
        } else {
            status = panic("unexpected map");
        }
    } else if(type == event_scalar) {
        if(node->type & string) {
            if(parser->callback(next, node->mark, value, parser->context))
                status = panic("failed to process next event");
        } else {
            status = panic("unexpected string");
        }
    } else {
        status = panic("invalid type - %d", type);
    }

    return status;
}

int parser_event(enum event_type type, struct string * value, void * context) {
    int status = 0;
    struct parser * parser;
    struct schema_node * node;

    parser = context;
    node = parser->data;
    if(node) {
        if(parser_start(parser, node, type, value)) {
            status = panic("failed to start parser object");
        } else {
            parser->data = NULL;
        }
    } else {
        node = parser->root;
        if(!node) {
            status = panic("invalid node");
        } else if(node->state == list) {
            if(type == event_list_end) {
                if(parser->callback(end, node->mark, NULL, parser->context)) {
                    status = panic("failed to process end event");
                } else {
                    parser->root->state = 0;
                    parser->root = parser->root->next;
                }
            } else if(parser_start(parser, node->list, type, value)) {
                status = panic("failed to start parser object");
            }
        } else if(node->state == map) {
            if(type == event_map_end) {
                if(parser->callback(end, node->mark, NULL, parser->context)) {
                    status = panic("failed to process end event");
                } else {
                    parser->root->state = 0;
                    parser->root = parser->root->next;
                }
            } else if(type == event_scalar) {
                parser->data = map_search(&node->map, value->string);
                if(!parser->data)
                    status = panic("invalid key - %s", value->string);
            } else {
                status = panic("invalid type - %d", type);
            }
        } else {
            status = panic("invalid node state - %d", node->type);
        }
    }

    return status;
}

int parser_parse(struct parser * parser, struct schema * schema, parser_cb callback, void * context, const char * path) {
    int status = 0;
    char * ext;

    parser->root = schema_top(schema);
    parser->data = NULL;
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

        if(parser->root != schema_top(schema))
            status = panic("failed to parse %s", path);
    }

    return status;
}
