#include "yaml_reader.h"

struct yaml_read {
    struct meta * meta;
    struct meta_node * root;
    struct meta_node * node;
    yaml_reader_cb cb;
    void * arg;
};

static inline int yaml_reader_push(struct yaml_read * read, enum yaml_event event, struct meta_node * node) {
    if(event == yaml_map_start) {
        node->scope = meta_map;
        read->root = node;

        if(node->type & meta_map)
            return read->cb(event, node->id, NULL, 0, read->arg);
    } else if(event == yaml_list_start) {
        node->scope = meta_list;
        read->root = node;

        if(node->type & meta_list)
            return read->cb(event, node->id, NULL, 0, read->arg);
    } else {
        return panic("invalid event - %d", event);
    }

    return 0;
}

static inline int yaml_reader_pop(struct yaml_read * read, enum yaml_event event, int type) {
    struct meta_node * node;

    node = read->root;
    read->root = node->prev;

    if(node->type & type)
        return read->cb(event, node->id, NULL, 0, read->arg);

    return 0;
}

int yaml_reader_event(enum yaml_event event, char * string, size_t length, void * arg) {
    struct yaml_read * read = arg;
    struct meta_node * node;

    if(read->root->scope == meta_map) {
        if(read->node) {
            node = read->node;
            read->node = NULL;

            if(event == yaml_string) {
                if(node->type & meta_string)
                    return read->cb(event, node->id, string, length, read->arg);
            } else {
                return yaml_reader_push(read, event, node);
            }
        } else {
            if(event == yaml_map_end) {
                yaml_reader_pop(read, event, meta_map);
            } else if(event == yaml_string) {
                read->node = meta_add(read->meta, read->root, string, length);
                if(!read->node)
                    return panic("failed to add meta object");
            } else {
                return panic("invalid event - %d", event);
            }
        }
    } else {
        if(event == yaml_list_end) {
            yaml_reader_pop(read, event, meta_list);
        } else {
            node = meta_add(read->meta, read->root, NULL, 0);
            if(!node)
                return panic("failed to add meta object");

            if(event == yaml_string) {
                if(node->type & meta_string)
                    return read->cb(event, node->id, string, length, read->arg);
            } else {
                return yaml_reader_push(read, event, node);
            }
        }
    }

    return 0;
}

int yaml_reader_parse(struct yaml * yaml, const char * path, struct meta * meta, yaml_reader_cb cb, void * arg) {
    struct yaml_read read;

    read.meta = meta;
    read.root = NULL;
    read.node = NULL;
    read.cb = cb;
    read.arg = arg;

    if(yaml_reader_push(&read, yaml_list_start, meta->root))
        return panic("failed to push read object");

    if(yaml_parse(yaml, path, yaml_reader_event, &read))
        return panic("failed to parse yaml object");

    return 0;
}
