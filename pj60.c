#include "yaml.h"

enum type {
    list = 0x1,
    map = 0x2,
    str = 0x4
};

struct node {
    enum type type;
    enum type state;
    struct node * list;
    struct map * map;
    struct node * next;
};

struct data {
    struct heap * heap;
    struct store store;
    struct strbuf strbuf;
    struct node * root;
    char * key;
    int mark;
};

struct node * node_create(struct data *, enum type);
void node_destroy(struct node *);
void node_print(struct node *, int, char *);

int data_create(struct data *, size_t, struct heap *);
void data_destroy(struct data *);
int data_parse(enum event_type, struct string *, void *);
void data_print(struct data *);
void data_markup(struct data *, char *);
void data_markup_loop(struct data *, struct node *, int, char *);

static inline struct node * data_node_top(struct data *);
static inline void data_node_push(struct data *, struct node *, enum type);
static inline void data_node_pop(struct data *);

static inline char * data_key_top(struct data *);
static inline int data_key_push(struct data *, struct string *);
static inline void data_key_pop(struct data *);
static inline char * data_key_store(struct data *);

int main(int argc, char ** argv) {
    int status = 0;
    struct heap heap;
    struct yaml yaml;
    struct data data;

    if(argc < 2) {
        status = panic("%s <path> <name>", argv[0]);
    } else if(heap_create(&heap, 4096)) {
        status = panic("failed to create heap object");
    } else {
        if(yaml_create(&yaml, 4096, &heap)) {
            status = panic("failed to create yaml object");
        } else {
            if(data_create(&data, 4096, &heap)) {
                status = panic("failed to create data object");
            } else {
                if(yaml_parse(&yaml, argv[1], 4096, data_parse, &data)) {
                    status = panic("failed to parse yaml object");
                } else {
                    if(argc == 3) {
                        data_markup(&data, argv[2]);
                    } else {
                        data_print(&data);
                    }
                }
                data_destroy(&data);
            }
            yaml_destroy(&yaml);
        }
        heap_destroy(&heap);
    }

    return status;
}

struct node * node_create(struct data * data, enum type type) {
    int status = 0;
    struct node * node;

    node = store_malloc(&data->store, sizeof(*node));
    if(!node) {
        status = panic("failed to malloc store object");
    } else {
        node->type = type;
        node->state = 0;
        node->list = NULL;
        node->map = store_malloc(&data->store, sizeof(*node->map));
        if(!node->map) {
            status = panic("failed to malloc store object");
        } else if(map_create(node->map, (map_compare_cb) strcmp, data->heap->map_pool)) {
            status = panic("failed to create map object");
        }
        node->next = NULL;
    }

    return status ? NULL : node;
}

void node_destroy(struct node * node) {
    struct map_kv kv;

    kv = map_start(node->map);
    while(kv.key) {
        node_destroy(kv.value);
        kv = map_next(node->map);
    }

    if(node->list)
        node_destroy(node->list);

    map_destroy(node->map);
}

void node_print(struct node * node, int scope, char * key) {
    int i;
    struct map_kv kv;

    for(i = 0; i < scope; i++)
        fputs("    ", stdout);

    if(key)
        fprintf(stdout, "[%s]", key);

    switch(node->type) {
        case list | map | str:
            fprintf(stdout, "[list, map, string]\n");
            break;
        case list | map:
            fprintf(stdout, "[list, map]\n");
            break;
        case list | str:
            fprintf(stdout, "[list, string]\n");
            break;
        case map | str:
            fprintf(stdout, "[map, string]\n");
            break;
        case list:
            fprintf(stdout, "[list]\n");
            break;
        case map:
            fprintf(stdout, "[map]\n");
            break;
        case str:
            fprintf(stdout, "[string]\n");
            break;
    }

    if(node->type & list)
        node_print(node->list, scope + 1, NULL);

    if(node->type & map) {
        kv = map_start(node->map);
        while(kv.key) {
            node_print(kv.value, scope + 1, kv.key);
            kv = map_next(node->map);
        }
    }
}

int data_create(struct data * data, size_t size, struct heap * heap) {
    int status = 0;

    data->heap = heap;
    if(!data->heap) {
        status = panic("invalid heap");
    } else {
        if(store_create(&data->store, size)) {
            status = panic("failed to create store object");
        } else {
            if(strbuf_create(&data->strbuf, size)) {
                status = panic("failed to create strbuf object");
            } else {
                data->root = node_create(data, list);
                if(!data->root) {
                    status = panic("failed to create node object");
                } else {
                    data->root->state = list;
                    data->key = NULL;
                    data->mark = 0;
                }
                if(status)
                    strbuf_destroy(&data->strbuf);
            }
            if(status)
                store_destroy(&data->store);
        }
    }

    return status;
}

void data_destroy(struct data * data) {
    while(data->root->next)
        data->root = data->root->next;

    node_destroy(data->root);
    strbuf_destroy(&data->strbuf);
    store_destroy(&data->store);
}

int data_parse(enum event_type type, struct string * string, void * data) {
    int status = 0;
    struct node * node;

    char * key;
    struct node * value;

    node = data_node_top(data);
    if(!node) {
        status = panic("invalid node");
    } else {
        key = data_key_top(data);
        if(key) {
            if(node->state != map) {
                status = panic("expected map");
            } else {
                if(type == list_begin) {
                    value = map_search(node->map, key);
                    if(value) {
                        value->type |= list;
                        data_node_push(data, value, list);
                    } else {
                        value = node_create(data, list);
                        if(!value) {
                            status = panic("failed to create node object");
                        } else {
                            key = data_key_store(data);
                            if(!key) {
                                status = panic("failed to key store data object");
                            } else if(map_insert(node->map, key, value)) {
                                status = panic("failed to insert map object");
                            } else {
                                data_node_push(data, value, list);
                            }
                            if(status)
                                node_destroy(value);
                        }
                    }
                } else if(type == map_begin) {
                    value = map_search(node->map, key);
                    if(value) {
                        value->type |= map;
                        data_node_push(data, value, map);
                    } else {
                        value = node_create(data, map);
                        if(!value) {
                            status = panic("failed to create node object");
                        } else {
                            key = data_key_store(data);
                            if(!key) {
                                status = panic("failed to key store data object");
                            } else if(map_insert(node->map, key, value)) {
                                status = panic("failed to insert map object");
                            } else {
                                data_node_push(data, value, map);
                            }
                            if(status)
                                node_destroy(value);
                        }
                    }
                } else if(type == scalar) {
                    value = map_search(node->map, key);
                    if(value) {
                        value->type |= str;
                    } else {
                        value = node_create(data, str);
                        if(!value) {
                            status = panic("failed to create node object");
                        } else {
                            key = data_key_store(data);
                            if(!key) {
                                status = panic("failed to key store data object");
                            } else if(map_insert(node->map, key, value)) {
                                status = panic("failed to insert map object");
                            }
                            if(status)
                                node_destroy(value);
                        }
                    }
                } else {
                    status = panic("invalid type - %d", type);
                }
            }
            data_key_pop(data);
        } else if(node->state == list) {
            if(type == list_end) {
                data_node_pop(data);
            } else if(type == list_begin) {
                if(node->list) {
                    node->list->type |= list;
                    data_node_push(data, node->list, list);
                } else {
                    node->list = node_create(data, list);
                    if(!node->list) {
                        status = panic("failed to create node object");
                    } else {
                        data_node_push(data, node->list, list);
                    }
                }
            } else if(type == map_begin) {
                if(node->list) {
                    node->list->type |= map;
                    data_node_push(data, node->list, map);
                } else {
                    node->list = node_create(data, map);
                    if(!node->list) {
                        status = panic("failed to create node object");
                    } else {
                        data_node_push(data, node->list, map);
                    }
                }
            } else if(type == scalar) {
                if(node->list) {
                    node->list->type |= str;
                } else {
                    node->list = node_create(data, str);
                    if(!node->list)
                        status = panic("failed to create node object");
                }
            } else {
                status = panic("invalid type - %d", type);
            }
        } else if(node->state == map) {
            if(type == map_end) {
                data_node_pop(data);
            } else if(type == scalar) {
                if(data_key_push(data, string))
                    status = panic("failed to key push data object");
            } else {
                status = panic("invalid type - %d", type);
            }
        }
    }

    return status;
}

void data_print(struct data * data) {
    if(data->root->list)
        node_print(data->root->list, 0, NULL);
}

void data_markup(struct data * data, char * name) {
    fprintf(stdout, "struct schema_markup %s_markup[] = {\n", name);

    if(data->root->list)
        data_markup_loop(data, data->root->list, 1, NULL);

    fprintf(stdout, "    {0, 0, 0}\n};\n");
}

void data_markup_loop(struct data * data, struct node * node, int scope, char * key) {
    struct map_kv kv;

    fprintf(stdout, "    {%d, ", scope);

    switch(node->type) {
        case list | map | str:
            fprintf(stdout, "list | map | string, %d", data->mark++);
            break;
        case list | map:
            fprintf(stdout, "list | map, %d", data->mark++);
            break;
        case list | str:
            fprintf(stdout, "list | string, %d", data->mark++);
            break;
        case map | str:
            fprintf(stdout, "map | string, %d", data->mark++);
            break;
        case list:
            fprintf(stdout, "list, %d", data->mark++);
            break;
        case map:
            fprintf(stdout, "map, %d", data->mark++);
            break;
        case str:
            fprintf(stdout, "string, %d", data->mark++);
            break;
    }

    if(key) {
        fprintf(stdout, ", \"%s\"},\n", key);
    } else {
        fprintf(stdout, ", NULL},\n");
    }

    if(node->type & list)
        data_markup_loop(data, node->list, scope + 1, NULL);

    if(node->type & map) {
        kv = map_start(node->map);
        while(kv.key) {
            data_markup_loop(data, kv.value, scope + 1, kv.key);
            kv = map_next(node->map);
        }
    }
}

static inline struct node * data_node_top(struct data * data) {
    return data->root;
}

static inline void data_node_push(struct data * data, struct node * node, enum type state) {
    node->state = state;
    node->next = data->root;
    data->root = node;
}

static inline void data_node_pop(struct data * data) {
    data->root->state = 0;
    data->root = data->root->next;
}

static inline char * data_key_top(struct data * data) {
    return data->key;
}

static inline int data_key_push(struct data * data, struct string * string) {
    int status = 0;

    if(strbuf_strcpy(&data->strbuf, string->string, string->length)) {
        status = panic("failed to strcpy strbuf object");
    } else {
        data->key = strbuf_char(&data->strbuf);
        if(!data->key)
            status = panic("failed to string strbuf object");
    }

    return status;
}

static inline void data_key_pop(struct data * data) {
    data->key = NULL;
    strbuf_clear(&data->strbuf);
}

static inline char * data_key_store(struct data * data) {
    int status = 0;
    char * key;

    if(!data->key) {
        status = panic("invalid key");
    } else {
        key = store_printf(&data->store, "%s", data->key);
        if(!key)
            status = panic("failed to printf store object");
    }

    return status ? NULL : key;
}
