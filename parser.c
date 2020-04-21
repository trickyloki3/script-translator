#include "parser.h"

struct schema_node * schema_node_create(struct schema *, enum schema_type, int);
void schema_node_destroy(struct schema *, struct schema_node *);
void schema_node_print(struct schema_node *, int, char *);

struct parser_state_node {
    enum schema_type type;
    struct schema_node * data;
    struct parser_state_node * next;
};

struct parser_state {
    struct pool * pool;
    struct parser_state_node * root;
    struct schema_node * data;
    parser_cb callback;
    void * context;
};

int parser_state_push(struct parser_state *, struct schema_node *, enum schema_type);
void parser_state_pop(struct parser_state *);
void parser_state_clear(struct parser_state *);
int parser_state_start(struct parser_state *, struct schema_node *, enum event_type, struct string *);
int parser_state_event(enum event_type, struct string *, void *);

int parser_parse_path(struct parser *, struct parser_state *, const char *);

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
        } else if(map_create(node->map, (map_compare_cb) strcmp, &schema->pool)) {
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
            fprintf(stdout, "[list | map | string][%d]\n", node->mark);
            break;
        case list | map:
            fprintf(stdout, "[list | map][%d]\n", node->mark);
            break;
        case list | string:
            fprintf(stdout, "[list | string][%d]\n", node->mark);
            break;
        case map | string:
            fprintf(stdout, "[map | string][%d]\n", node->mark);
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

int schema_create(struct schema * schema, size_t size) {
    int status = 0;

    if(pool_create(&schema->pool, sizeof(struct map_node), size / sizeof(struct map_node))) {
        status = panic("failed to create pool object");
    } else {
        if(store_create(&schema->store, size)) {
            status = panic("failed to create store object");
        } else {
            schema->root = NULL;

            if(status)
                store_destroy(&schema->store);
        }
        if(status)
            pool_destroy(&schema->pool);
    }

    return status;
}

void schema_destroy(struct schema * schema) {
    schema_clear(schema);
    store_destroy(&schema->store);
    pool_destroy(&schema->pool);
}

void schema_clear(struct schema * schema) {
    struct schema_node * node;

    if(schema->root) {
        node = schema->root;
        schema->root = NULL;
        schema_node_destroy(schema, node);
    }

    store_clear(&schema->store);
}

void schema_print(struct schema * schema) {
    if(schema->root)
        schema_node_print(schema->root, 0, NULL);
}

int schema_load(struct schema * schema, struct schema_markup * array) {
    int status = 0;
    struct schema_markup * scope = NULL;

    char * key;
    struct schema_node * node;
    struct schema_node * root;

    schema_clear(schema);

    schema->root = schema_node_create(schema, list, 0);
    if(!schema->root) {
        status = panic("failed to create schema node object");
    } else {
        scope = store_calloc(&schema->store, sizeof(*scope));
        if(!scope) {
            status = panic("failed to calloc store object");
        } else {
            scope->node = schema->root;

            while(array->level > 0 && !status) {
                while(scope->level >= array->level)
                    scope = scope->next;

                node = schema_node_create(schema, array->type, array->mark);
                if(!node) {
                    status = panic("failed to create schema node object");
                } else {
                    root = scope->node;
                    if(!root) {
                        status = panic("invalid root");
                    } else {
                        if(array->key) {
                            if(root->type & map) {
                                key = store_printf(&schema->store, "%s", array->key);
                                if(!key) {
                                    status = panic("failed to strcpy store object");
                                } else if(map_insert(root->map, key, node)) {
                                    status = panic("failed to insert map object");
                                }
                            } else {
                                status = panic("expected map");
                            }
                        } else {
                            if(root->type & list) {
                                if(root->list) {
                                    status = panic("invalid list");
                                } else {
                                    root->list = node;
                                }
                            } else {
                                status = panic("expected list");
                            }
                        }
                    }

                    if(status) {
                        schema_node_destroy(schema, node);
                    } else if(node->type & (list | map)) {
                        array->node = node;
                        array->next = scope;
                        scope = array;
                    }
                }

                array++;
            }
        }
    }

    return status;
}

int parser_state_push(struct parser_state * state, struct schema_node * data, enum schema_type type) {
    int status = 0;
    struct parser_state_node * node;

    node = pool_get(state->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->type = type;
        node->data = data;
        node->next = state->root;
        state->root = node;
    }

    return status;
}

void parser_state_pop(struct parser_state * state) {
    struct parser_state_node * node;

    node = state->root;
    state->root = state->root->next;
    pool_put(state->pool, node);
}

void parser_state_clear(struct parser_state * state) {
    struct parser_state_node * node;

    while(state->root) {
        node = state->root;
        state->root = state->root->next;
        pool_put(state->pool, node);
    }
}

int parser_state_start(struct parser_state * state, struct schema_node * node, enum event_type type, struct string * value) {
    int status = 0;

    if(type == event_list_start) {
        if(node->type & list) {
            if(state->callback(start, node->mark, NULL, state->context)) {
                status = panic("failed to process start event");
            } else if(parser_state_push(state, node, list)) {
                status = panic("failed to push parser state object");
            }
        } else {
            status = panic("unexpected list");
        }
    } else if(type == event_map_start) {
        if(node->type & map) {
            if(state->callback(start, node->mark, NULL, state->context)) {
                status = panic("failed to process start event");
            } else if(parser_state_push(state, node, map)) {
                status = panic("failed to push parser state object");
            }
        } else {
            status = panic("unexpected map");
        }
    } else if(type == event_scalar) {
        if(node->type & string) {
            if(state->callback(next, node->mark, value, state->context))
                status = panic("failed to process next event");
        } else {
            status = panic("unexpected string");
        }
    } else {
        status = panic("invalid type - %d", type);
    }

    return status;
}

/*
 * parser_state_event grammar
 *
 *  node : string
 *       | start list end
 *       | start map end
 *
 *  list : node
 *       | list node
 *
 *  map  : string node
 *       | map string node
 */

int parser_state_event(enum event_type type, struct string * value, void * context) {
    int status = 0;
    struct parser_state * state;
    struct parser_state_node * node;

    state = context;

    if(state->data) {
        if(parser_state_start(state, state->data, type, value)) {
            status = panic("failed to start parser state object");
        } else {
            state->data = NULL;
        }
    } else {
        node = state->root;
        if(!node) {
            status = panic("invalid node");
        } else if(node->type == list) {
            if(type == event_list_end) {
                if(state->callback(end, node->data->mark, NULL, state->context)) {
                    status = panic("failed to process end event");
                } else {
                    parser_state_pop(state);
                }
            } else if(parser_state_start(state, node->data->list, type, value)) {
                status = panic("failed to start parser state object");
            }
        } else if(node->type == map) {
            if(type == event_map_end) {
                if(state->callback(end, node->data->mark, NULL, state->context)) {
                    status = panic("failed to process end event");
                } else {
                    parser_state_pop(state);
                }
            } else if(type == event_scalar) {
                state->data = map_search(node->data->map, value->string);
                if(!state->data)
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

int parser_create(struct parser * parser, size_t size, struct heap * heap) {
    int status = 0;

    parser->size = size;

    parser->pool = heap_pool(heap, sizeof(struct parser_state_node));
    if(!parser->pool) {
        status = panic("failed to pool heap object");
    } else {
        if(csv_create(&parser->csv, parser->size, heap)) {
            status = panic("failed to create csv object");
        } else {
            if(json_create(&parser->json, parser->size)) {
                status = panic("failed to create json object");
            } else {
                if(yaml_create(&parser->yaml, parser->size, heap)) {
                    status = panic("failed to create yaml object");
                } else {

                    if(status)
                        yaml_destroy(&parser->yaml);
                }
                if(status)
                    json_destroy(&parser->json);
            }
            if(status)
                csv_destroy(&parser->csv);
        }
    }

    return status;
}

void parser_destroy(struct parser * parser) {
    yaml_destroy(&parser->yaml);
    json_destroy(&parser->json);
    csv_destroy(&parser->csv);
}

int parser_parse(struct parser * parser, struct schema * schema, parser_cb callback, void * context, const char * path) {
    int status = 0;

    struct parser_state state;

    state.pool = parser->pool;
    state.root = NULL;
    state.data = NULL;
    state.callback = callback;
    state.context = context;

    if(parser_state_push(&state, schema->root, list)) {
        status = panic("failed to push parser state object");
    } else {
        if(parser_parse_path(parser, &state, path))
            status = panic("failed to parse path parser object");

        parser_state_clear(&state);
    }

    return status;
}

int parser_parse_path(struct parser * parser, struct parser_state * state, const char * path) {
    int status = 0;

    char * ext;

    ext = strrchr(path, '.');
    if(!ext) {
        status = panic("failed to get file extension - %s", path);
    } else {
        if(!strcmp(ext, ".txt")) {
            if(csv_parse(&parser->csv, path, parser->size, parser_state_event, state))
                status = panic("failed to parse csv object");
        } else if(!strcmp(ext, ".json")) {
            if(json_parse(&parser->json, path, parser->size, parser_state_event, state))
                status = panic("failed to parse json object");
        } else if(!strcmp(ext, ".yaml") || !strcmp(ext, ".yml")) {
            if(yaml_parse(&parser->yaml, path, parser->size, parser_state_event, state))
                status = panic("failed to parse yaml object");
        }
    }

    return status;
}
