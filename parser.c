#include "parser.h"

struct schema_node * schema_node_create(struct schema *, enum schema_type, int);
void schema_node_destroy(struct schema *, struct schema_node *);
void schema_node_print(struct schema_node *, int, char *);

void schema_clear(struct schema *);
int schema_reset(struct schema *);

struct schema_node * schema_add(struct schema *, struct schema_node *, enum schema_type, int, char *);
struct schema_node * schema_get(struct schema_node *, char *);

struct schema_state {
    struct schema * schema;
    struct schema_node * root;
    struct strbuf * strbuf;
    char * key;
};

int schema_state_parse(enum event_type, struct string *, void *);
int schema_state_node(struct schema_state *, enum event_type, char *);

struct data_state {
    struct schema_node * root;
    struct schema_node * data;
    parser_cb callback;
    void * context;
};

int data_state_parse(enum event_type, struct string *, void *);
int data_state_node(struct data_state *, struct schema_node *, enum event_type, struct string *);

int parser_parse(struct parser *, const char *, event_cb, void *);

struct schema_node * schema_node_create(struct schema * schema, enum schema_type type, int mark) {
    int status = 0;
    struct schema_node * node;

    node = store_calloc(&schema->store, sizeof(*node));
    if(!node) {
        status = panic("failed to calloc store object");
    } else {
        node->type = type;
        node->mark = mark;
        if(map_create(&node->map, (map_compare_cb) strcmp, &schema->pool))
            status = panic("failed to create map object");
    }

    return status ? NULL : node;
}

void schema_node_destroy(struct schema * schema, struct schema_node * node) {
    struct map_kv kv;

    if(node->list)
        schema_node_destroy(schema, node->list);

    kv = map_start(&node->map);
    while(kv.key) {
        schema_node_destroy(schema, kv.value);
        kv = map_next(&node->map);
    }

    map_destroy(&node->map);
}

void schema_node_print(struct schema_node * node, int indent, char * key) {
    int i;
    struct map_kv kv;

    for(i = 0; i < indent; i++)
        fputs("  ", stdout);

    if(key)
        fprintf(stdout, "[%s]", key);

    switch(node->type) {
        case schema_list | schema_map | schema_string:
            fprintf(stdout, "[list | map | string]");
            break;
        case schema_list | schema_map:
            fprintf(stdout, "[list | map]");
            break;
        case schema_list | schema_string:
            fprintf(stdout, "[list | string]");
            break;
        case schema_map | schema_string:
            fprintf(stdout, "[map | string]");
            break;
        case schema_list:
            fprintf(stdout, "[list]");
            break;
        case schema_map:
            fprintf(stdout, "[map]");
            break;
        case schema_string:
            fprintf(stdout, "[string]");
            break;
    }

    fprintf(stdout, "[%d]\n", node->mark);

    if(node->type & schema_map) {
        kv = map_start(&node->map);
        while(kv.key) {
            schema_node_print(kv.value, indent + 1, kv.key);
            kv = map_next(&node->map);
        }
    }

    if(node->type & schema_list && node->list)
        schema_node_print(node->list, indent + 1, NULL);
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

int schema_reset(struct schema * schema) {
    int status = 0;

    schema_clear(schema);

    schema->root = schema_node_create(schema, schema_list, 0);
    if(!schema->root)
        status = panic("failed to create schema node object");

    return status;
}

void schema_print(struct schema * schema) {
    if(schema->root)
        schema_node_print(schema->root, 0, NULL);
}

struct schema_node * schema_add(struct schema * schema, struct schema_node * root, enum schema_type type, int mark, char * key) {
    int status = 0;
    struct schema_node * node;

    if(key) {
        if(root->type & schema_map) {
            node = map_search(&root->map, key);
            if(node) {
                node->type |= type;
            } else {
                node = schema_node_create(schema, type, mark);
                if(!node) {
                    status = panic("failed to create schema node object");
                } else {
                    key = store_strcpy(&schema->store, key, strlen(key));
                    if(!key) {
                        status = panic("failed to strcpy store object");
                    } else if(map_insert(&root->map, key, node)) {
                        status = panic("failed to insert map object");
                    }
                    if(status)
                        schema_node_destroy(schema, node);
                }
            }
        } else {
            status = panic("expected map");
        }
    } else {
        if(root->type & schema_list) {
            node = root->list;
            if(node) {
                node->type |= type;
            } else {
                node = schema_node_create(schema, type, mark);
                if(!node) {
                    status = panic("failed to create schema node object");
                } else {
                    root->list = node;
                }
            }
        } else {
            status = panic("expected list");
        }
    }

    return status ? NULL : node;
}

struct schema_node * schema_get(struct schema_node * root, char * key) {
    int status = 0;
    struct schema_node * node;

    if(key) {
        if(root->type & schema_map) {
            node = map_search(&root->map, key);
            if(!node)
                status = panic("invalid key - %s", key);
        } else {
            status = panic("expected map");
        }
    } else {
        if(root->type & schema_list) {
            node = root->list;
            if(!node)
                status = panic("invalid node");
        } else {
            status = panic("expected list");
        }
    }

    return status ? NULL : node;
}

int schema_reload(struct schema * schema, struct schema_markup * array) {
    int status = 0;
    struct schema_node * node;
    struct schema_node * root;

    if(schema_reset(schema)) {
        status = panic("failed to reset schema object");
    } else {
        root = schema->root;
        if(!root) {
            status = panic("invalid node");
        } else {
            root->state = 0;

            while(array->level > 0 && !status) {
                while(root->state >= array->level)
                    root = root->next;

                node = schema_add(schema, root, array->type, array->mark, array->key);
                if(!node) {
                    status = panic("failed to add schema object");
                } else {
                    if(array->type & (schema_list | schema_map)) {
                        node->state = array->level;
                        node->next = root;
                        root = node;
                    }

                    array++;
                }
            }
        }
    }

    return status;
}

int schema_update(struct schema * schema, struct schema_markup * array) {
    int status = 0;
    struct schema_node * node;
    struct schema_node * root;

    root = schema->root;
    if(!root) {
        status = panic("invalid node");
    } else {
        root->state = 0;

        while(array->level > 0 && !status) {
            while(root->state >= array->level)
                root = root->next;

            node = schema_get(root, array->key);
            if(!node) {
                status = panic("failed to get schema object");
            } else {
                if(node->type & array->type) {
                    node->mark = array->mark;

                    if(array->type & (schema_list | schema_map)) {
                        node->state = array->level;
                        node->next = root;
                        root = node;
                    }

                    array++;
                } else {
                    status = panic("expected %d", array->type);
                }
            }
        }
    }

    return status;
}

/*
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

int schema_state_parse(enum event_type type, struct string * string, void * context) {
    int status = 0;
    struct schema_state * state = context;

    if(state->root->state == schema_list) {
        if(type == event_list_end) {
            state->root = state->root->next;
        } else if(schema_state_node(state, type, NULL)) {
            status = panic("failed to node schema state object");
        }
    } else if(state->root->state == schema_map) {
        if(state->key) {
            if(schema_state_node(state, type, state->key))
                status = panic("failed to node schema state object");
            state->key = NULL;
            strbuf_clear(state->strbuf);
        } else if(type == event_scalar) {
            if(strbuf_strcpy(state->strbuf, string->string, string->length)) {
                status = panic("failed to strcpy strbuf object");
            } else {
                state->key = strbuf_array(state->strbuf);
                if(!state->key)
                    status = panic("failed to array strbuf object");
            }
        } else if(type == event_map_end) {
            state->root = state->root->next;
        } else {
            status = panic("invalid type - %d", type);
        }
    } else {
        status = panic("invalid node state -- %d", state->root->state);
    }

    return status;
}

int schema_state_node(struct schema_state * state, enum event_type type, char * key) {
    int status = 0;
    struct schema_node * node;

    if(type == event_list_start) {
        node = schema_add(state->schema, state->root, schema_list, 0, key);
        if(!node) {
            status = panic("failed to add schema object");
        } else {
            node->state = schema_list;
            node->next = state->root;
            state->root = node;
        }
    } else if(type == event_map_start) {
        node = schema_add(state->schema, state->root, schema_map, 0, key);
        if(!node) {
            status = panic("failed to add schema object");
        } else {
            node->state = schema_map;
            node->next = state->root;
            state->root = node;
        }
    } else if(type == event_scalar) {
        node = schema_add(state->schema, state->root, schema_string, 0, key);
        if(!node)
            status = panic("failed to add schema object");
    } else {
        status = panic("invalid type - %d", type);
    }

    return status;
}

int data_state_parse(enum event_type type, struct string * string, void * context) {
    int status = 0;

    struct data_state * state;
    struct schema_node * node;

    state = context;

    if(state->root->state == schema_list) {
        if(type == event_list_end) {
            if(state->callback(parser_end, state->root->mark, NULL, state->context)) {
                status = panic("failed to process end event");
            } else {
                state->root = state->root->next;
            }
        } else {
            node = schema_get(state->root, NULL);
            if(!node) {
                status = panic("failed to get schema object");
            } else if(data_state_node(state, node, type, string)) {
                status = panic("failed to node parser state object");
            }
        }
    } else if(state->root->state == schema_map) {
        if(state->data) {
            if(data_state_node(state, state->data, type, string)) {
                status = panic("failed to node parser state object");
            } else {
                state->data = NULL;
            }
        } else if(type == event_scalar) {
            node = schema_get(state->root, string->string);
            if(!node) {
                status = panic("failed to get schema object");
            } else {
                state->data = node;
            }
        } else if(type == event_map_end) {
            if(state->callback(parser_end, state->root->mark, NULL, state->context)) {
                status = panic("failed to process end event");
            } else {
                state->root = state->root->next;
            }
        } else {
            status = panic("invalid type - %d", type);
        }
    } else {
        status = panic("invalid node state - %d", state->root->state);
    }

    return status;
}

int data_state_node(struct data_state * state, struct schema_node * node, enum event_type type, struct string * string) {
    int status = 0;

    if(type == event_list_start) {
        if(node->type & schema_list) {
            if(state->callback(parser_start, node->mark, NULL, state->context)) {
                status = panic("failed to process start event");
            } else {
                node->state = schema_list;
                node->next = state->root;
                state->root = node;
            }
        } else {
            status = panic("unexpected list");
        }
    } else if(type == event_map_start) {
        if(node->type & schema_map) {
            if(state->callback(parser_start, node->mark, NULL, state->context)) {
                status = panic("failed to process start event");
            } else {
                node->state = schema_map;
                node->next = state->root;
                state->root = node;
            }
        } else {
            status = panic("unexpected map");
        }
    } else if(type == event_scalar) {
        if(node->type & schema_string) {
            if(state->callback(parser_next, node->mark, string, state->context))
                status = panic("failed to process next event");
        } else {
            status = panic("unexpected string");
        }
    } else {
        status = panic("invalid type - %d", type);
    }

    return status;
}

int parser_create(struct parser * parser, size_t size) {
    int status = 0;

    if(yaml_create(&parser->yaml, size)) {
        status = panic("failed to create yaml object");
    } else if(strbuf_create(&parser->strbuf, size)) {
        status = panic("failed to create strbuf object");
        goto strbuf_fail;
    } else if(schema_create(&parser->schema, size)) {
        status = panic("failed to create schema object");
        goto schema_fail;
    }

    return status;

schema_fail:
    strbuf_destroy(&parser->strbuf);
strbuf_fail:
    yaml_destroy(&parser->yaml);

    return status;
}

void parser_destroy(struct parser * parser) {
    schema_destroy(&parser->schema);
    strbuf_destroy(&parser->strbuf);
    yaml_destroy(&parser->yaml);
}

int parser_schema_parse(struct parser * parser, struct schema * schema, const char * path) {
    int status = 0;

    struct schema_state state;

    if(schema_reset(schema)) {
        status = panic("failed to reset schema object");
    } else {
        state.schema = schema;
        state.root = schema->root;
        state.strbuf = &parser->strbuf;
        state.key = NULL;

        if(!state.root) {
            status = panic("invalid node");
        } else {
            state.root->state = schema_list;

            if(parser_parse(parser, path, schema_state_parse, &state))
                status = panic("failed to parse parser object");
        }
    }

    return status;
}

int parser_data_parse(struct parser * parser, struct schema * schema, const char * path, parser_cb callback, void * context) {
    int status = 0;

    struct data_state state;

    state.root = schema->root;
    state.data = NULL;
    state.callback = callback;
    state.context = context;

    if(!state.root) {
        status = panic("invalid node");
    } else {
        state.root->state = schema_list;

        if(parser_parse(parser, path, data_state_parse, &state))
            status = panic("failed to parse parser object");
    }

    return status;
}

int parser_parse(struct parser * parser, const char * path, event_cb callback, void * context) {
    int status = 0;

    char * ext;

    ext = strrchr(path, '.');
    if(!ext) {
        status = panic("failed to get file extension - %s", path);
    } else {
        if(!strcmp(ext, ".yaml") || !strcmp(ext, ".yml")) {
            if(yaml_parse(&parser->yaml, path, callback, context))
                status = panic("failed to parse yaml object");
        } else {
            status = panic("unsupported extension - %s", ext);
        }
    }

    return status;
}

int parser_file(struct parser * parser, struct schema_markup * markup, const char * path, parser_cb callback, void * context) {
    int status = 0;

    if(schema_reload(&parser->schema, markup)) {
        status = panic("failed to load schema object");
    } else if(parser_data_parse(parser, &parser->schema, path, callback, context)) {
        status = panic("failed to data parse parser object");
    }

    return status;
}

int parser_file2(struct parser * parser, struct schema_markup * markup, const char * path, parser_cb callback, void * context) {
    int status = 0;

    if(parser_schema_parse(parser, &parser->schema, path)) {
        status = panic("failed to schema parse parser object");
    } else if(schema_update(&parser->schema, markup)) {
        status = panic("failed to mark schema object");
    } else if(parser_data_parse(parser, &parser->schema, path, callback, context)) {
        status = panic("failed to data parse parser object");
    }

    return status;
}
