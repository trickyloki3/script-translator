#include "yaml.h"

struct node {
    enum event_type type;
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
};

struct node * node_create(struct data *, enum event_type);
void node_destroy(struct node *);
void node_print(struct node *, int, char *);

int data_create(struct data *, size_t, struct heap *);
void data_destroy(struct data *);
int data_parse(enum event_type, struct string *, void *);
void data_print(struct data *);

static inline struct node * data_node_top(struct data *);
static inline int data_node_push(struct data *, struct node *);
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

    if(argc != 2) {
        status = panic("%s <path>", argv[0]);
    } else if(heap_create(&heap, 65536)) {
        status = panic("failed to create heap object");
    } else {
        if(yaml_create(&yaml, 65536, &heap)) {
            status = panic("failed to create yaml object");
        } else {
            if(data_create(&data, 65536, &heap)) {
                status = panic("failed to create data object");
            } else {
                if(yaml_parse(&yaml, argv[1], 65536, data_parse, &data)) {
                    status = panic("failed to parse yaml object");
                } else {
                    data_print(&data);
                }
                data_destroy(&data);
            }
            yaml_destroy(&yaml);
        }
        heap_destroy(&heap);
    }

    return status;
}

struct node * node_create(struct data * data, enum event_type type) {
    int status = 0;
    struct node * node;

    node = store_malloc(&data->store, sizeof(*node));
    if(!node) {
        status = panic("failed to malloc store object");
    } else {
        node->type = type;
        node->list = NULL;
        if(node->type == map_begin) {
            node->map = store_malloc(&data->store, sizeof(*node->map));
            if(!node->map) {
                status = panic("failed to malloc store object");
            } else if(map_create(node->map, (map_compare_cb) strcmp, data->heap->map_pool)) {
                status = panic("failed to create map object");
            }
        } else {
            node->map = NULL;
        }
        node->next = NULL;
    }

    return status ? NULL : node;
}

void node_destroy(struct node * node) {
    struct map_kv kv;

    if(node->map) {
        kv = map_start(node->map);
        while(kv.key) {
            node_destroy(kv.value);
            kv = map_next(node->map);
        }
        map_destroy(node->map);
    }

    if(node->list) {
        node_destroy(node->list);
    }
}

void node_print(struct node * node, int scope, char * key) {
    int i;
    struct map_kv kv;

    for(i = 0; i < scope; i++)
        fputs("    ", stdout);

    if(key)
        fprintf(stdout, "[%s]", key);

    switch(node->type) {
        case list_begin:
            fprintf(stdout, "[list]\n");
            node_print(node->list, scope + 1, NULL);
            break;
        case map_begin:
            fprintf(stdout, "[map]\n");
            kv = map_start(node->map);
            while(kv.key) {
                node_print(kv.value, scope + 1, kv.key);
                kv = map_next(node->map);
            }
            break;
        case scalar:
            fprintf(stdout, "[string]\n");
            break;
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
                data->root = node_create(data, list_begin);
                if(!data->root) {
                    status = panic("failed to create node object");
                } else {
                    data->key = NULL;
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
            if(node->type != map_begin) {
                status = panic("expected map");
            } else {
                if(type == list_begin) {
                    value = map_search(node->map, key);
                    if(value) {
                        if(value->type != type) {
                            status = panic("expected list - %s", key);
                        } else {
                            data_node_push(data, value);
                        }
                    } else {
                        value = node_create(data, type);
                        if(!value) {
                            status = panic("failed to create node object");
                        } else {
                            key = data_key_store(data);
                            if(!key) {
                                status = panic("failed to key store data object");
                            } else if(map_insert(node->map, key, value)) {
                                status = panic("failed to insert map object");
                            } else {
                                data_node_push(data, value);
                            }
                            if(status)
                                node_destroy(value);
                        }
                    }
                } else if(type == map_begin) {
                    value = map_search(node->map, key);
                    if(value) {
                        if(value->type != type) {
                            status = panic("expected map - %s", key);
                        } else {
                            data_node_push(data, value);
                        }
                    } else {
                        value = node_create(data, type);
                        if(!value) {
                            status = panic("failed to create node object");
                        } else {
                            key = data_key_store(data);
                            if(!key) {
                                status = panic("failed to key store data object");
                            } else if(map_insert(node->map, key, value)) {
                                status = panic("failed to insert map object");
                            } else {
                                data_node_push(data, value);
                            }
                            if(status)
                                node_destroy(value);
                        }
                    }
                } else if(type == scalar) {
                    value = map_search(node->map, key);
                    if(value) {
                        if(value->type != type)
                            status = panic("expected scalar - %s", key);
                    } else {
                        value = node_create(data, type);
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
        } else if(node->type == list_begin) {
            if(type == list_end) {
                data_node_pop(data);
            } else if(type == list_begin) {
                if(node->list) {
                    if(node->list->type != type) {
                        status = panic("expected list");
                    } else {
                        data_node_push(data, node->list);
                    }
                } else {
                    node->list = node_create(data, type);
                    if(!node->list) {
                        status = panic("failed to create node object");
                    } else {
                        data_node_push(data, node->list);
                    }
                }
            } else if(type == map_begin) {
                if(node->list) {
                    if(node->list->type != type) {
                        status = panic("expected map");
                    } else {
                        data_node_push(data, node->list);
                    }
                } else {
                    node->list = node_create(data, type);
                    if(!node->list) {
                        status = panic("failed to create node object");
                    } else {
                        data_node_push(data, node->list);
                    }
                }
            } else if(type == scalar) {
                if(node->list) {
                    if(node->list->type != type)
                        status = panic("expected scalar");
                } else {
                    node->list = node_create(data, type);
                    if(!node->list)
                        status = panic("failed to create node object");
                }
            } else {
                status = panic("invalid type - %d", type);
            }
        } else if(node->type == map_begin) {
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
    node_print(data->root, 0, NULL);
}

static inline struct node * data_node_top(struct data * data) {
    return data->root;
}

static inline int data_node_push(struct data * data, struct node * node) {
    node->next = data->root;
    data->root = node;
}

static inline void data_node_pop(struct data * data) {
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
