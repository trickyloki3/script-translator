#include "parser.h"

struct schema_node * schema_node_create(struct schema *, enum schema_type, int);
void schema_node_destroy(struct schema *, struct schema_node *);
void schema_node_print(struct schema_node *, int, char *);

static inline void schema_clear(struct schema *);
static inline void schema_push(struct schema *, struct schema_node *);
static inline void schema_pop(struct schema *);
static inline struct schema_node * schema_top(struct schema *);

int parser_start(struct parser *, struct schema_node *, enum event_type, struct string *);
int parser_event(enum event_type, struct string *, void *);

struct schema_node * schema_node_create(struct schema * schema, enum schema_type type, int mark) {
    int status = 0;
    struct schema_node * node;

    node = store_calloc(&schema->store, sizeof(*node));
    if(!node) {
        status = panic("failed to calloc store object");
    } else {
        node->type = type;
        node->mark = mark;
        node->map = store_malloc(&schema->store, sizeof(*node->map));
        if(!node->map) {
            status = panic("failed to malloc store object");
        } else if(map_create(node->map, (map_compare_cb) strcmp, schema->pool)) {
            status = panic("failed to create map object");
        }
    }

    return status ? NULL : node;
}

void schema_node_destroy(struct schema * schema, struct schema_node * node) {
    struct map_kv kv;

    kv = map_start(node->map);
    while(kv.key) {
        schema_node_destroy(schema, kv.value);
        kv = map_next(node->map);
    }

    if(node->list)
        schema_node_destroy(schema, node->list);

    map_destroy(node->map);
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
        kv = map_start(node->map);
        while(kv.key) {
            schema_node_print(kv.value, indent + 1, kv.key);
            kv = map_next(node->map);
        }
    }
}

int schema_create(struct schema * schema, size_t size, struct heap * heap) {
    int status = 0;

    schema->pool = heap->map_pool;
    if(!schema->pool) {
        status = panic("invalid pool");
    } else {
        if(store_create(&schema->store, size)) {
            status = panic("failed to create store object");
        } else {
            schema->root = NULL;
        }
    }

    return status;
}

void schema_destroy(struct schema * schema) {
    schema_clear(schema);
    store_destroy(&schema->store);
}

static inline void schema_clear(struct schema * schema) {
    struct schema_node * node;

    if(schema->root) {
        node = schema->root;
        schema->root = NULL;

        while(node->next)
            node = node->next;

        schema_node_destroy(schema, node);
    }

    store_clear(&schema->store);
}

static inline void schema_push(struct schema * schema, struct schema_node * node) {
    node->next = schema->root;
    schema->root = node;
}

static inline void schema_pop(struct schema * schema) {
    schema->root = schema->root->next;
}

static inline struct schema_node * schema_top(struct schema * schema) {
    return schema->root;
}

int schema_load(struct schema * schema, struct schema_markup * markup) {
    int status = 0;
    struct schema_markup * root = NULL;
    struct schema_node * node;

    schema_clear(schema);

    schema->root = schema_node_create(schema, list, 0);
    if(!schema->root) {
        status = panic("failed to create schema node object");
    } else {
        schema->root->state = list;

        while(markup->level && !status) {
            while(root && root->level >= markup->level) {
                schema_pop(schema);
                root = root->next;
            }

            node = schema_node_create(schema, markup->type, markup->mark);
            if(!node) {
                status = panic("failed to create schema node object");
            } else {
                if(markup->key) {
                    if(schema->root->type & map) {
                        if(map_insert(schema->root->map, markup->key, node))
                            status = panic("failed to insert map object");
                    } else {
                        status = panic("expected map");
                    }
                } else {
                    if(schema->root->type & list) {
                        if(schema->root->list) {
                            status = panic("invalid list");
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
                    schema_push(schema, node);
                    markup->next = root;
                    root = markup;
                }
            }

            markup++;
        }

        while(root) {
            schema_pop(schema);
            root = root->next;
        }
    }

    return status;
}

void schema_print(struct schema * schema) {
    struct schema_node * node;

    node = schema_top(schema);
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
                parser->data = map_search(node->map, value->string);
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
