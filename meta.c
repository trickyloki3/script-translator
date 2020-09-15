#include "meta.h"

struct meta_node * meta_node_create(struct meta *, struct meta_node *, char *, size_t);
void meta_node_destroy(struct meta *, struct meta_node *);
void meta_node_print(struct meta_node *, int, char *);

struct meta_node * meta_node_create(struct meta * meta, struct meta_node * root,  char * key, size_t size) {
    struct meta_node * node;

    node = zone_get(&meta->zone, sizeof(*node) + (size + 1));
    if(!node)
        return NULL;

    if(map_create(&node->map, (map_compare_cb) strcmp, &meta->pool))
        return NULL;

    node->scope = 0;
    node->type = 0;
    node->id = 0;
    node->key = (char *) (node + 1);
    node->list = NULL;
    node->prev = root;

    memcpy(node->key, key, size);
    node->key[size] = '\0';

    return node;
}

void meta_node_destroy(struct meta * meta, struct meta_node * node) {
    struct map_kv kv;

    kv = map_start(&node->map);
    while(kv.value) {
        meta_node_destroy(meta, kv.value);
        kv = map_next(&node->map);
    }

    if(node->list)
        meta_node_destroy(meta, node->list);

    map_destroy(&node->map);
}

void meta_node_print(struct meta_node * node, int indent, char * key) {
    int i;
    struct map_kv kv;

    for(i = 0; i < indent; i++)
        fputs("  ", stdout);

    if(key)
        fprintf(stdout, "(%s): ", key);

    fprintf(stdout, "[%d", node->id);

    if(node->type & meta_map)
        fputs(", map", stdout);

    if(node->type & meta_list)
        fputs(", list", stdout);

    if(node->type & meta_string)
        fputs(", string", stdout);

    fputs("]\n", stdout);

    kv = map_start(&node->map);
    while(kv.key) {
        meta_node_print(kv.value, indent + 1, kv.key);
        kv = map_next(&node->map);
    }

    if(node->list)
        meta_node_print(node->list, indent + 1, NULL);
}

int meta_create(struct meta * meta, size_t size) {
    int status = 0;

    if(pool_create(&meta->pool, sizeof(struct map_node), size / sizeof(struct map_node))) {
        status = panic("failed to create pool object");
    } else {
        if(zone_create(&meta->zone, size)) {
            status = panic("failed to create zone object");
        } else {
            meta->root = NULL;
        }
        if(status)
            pool_destroy(&meta->pool);
    }

    return status;
}

void meta_destroy(struct meta * meta) {
    meta_clear(meta);
    zone_destroy(&meta->zone);
    pool_destroy(&meta->pool);
}

void meta_clear(struct meta * meta) {
    struct meta_node * node;

    if(meta->root) {
        node = meta->root;
        meta->root = NULL;
        meta_node_destroy(meta, node);
    }

    zone_clear(&meta->zone);
}

void meta_print(struct meta * meta) {
    if(meta->root)
        meta_node_print(meta->root, 0, NULL);
}

struct meta_node * meta_add(struct meta * meta, struct meta_node * root, char * key, size_t size) {
    struct meta_node * node;

    if(key) {
        node = map_search(&root->map, key);
        if(node)
            return node;

        node = meta_node_create(meta, root, key, size);
        if(node) {
            if(map_insert(&root->map, node->key, node) == 0)
                return node;
            meta_node_destroy(meta, node);
        }
    } else {
        node = root->list;
        if(node)
            return node;

        node = meta_node_create(meta, root, NULL, 0);
        if(node)
            return root->list = node;
    }

    return NULL;
}

int meta_load(struct meta * meta, struct tag * tag) {
    struct meta_node * root;
    struct meta_node * node;

    meta_clear(meta);

    meta->root = meta_node_create(meta, NULL, NULL, 0);
    if(!meta->root)
        return panic("failed to create meta node object");

    root = meta->root;

    while(tag->scope > 0) {
        while(root->scope >= tag->scope)
            root = root->prev;

        node = meta_add(meta, root, tag->key, tag->key ? strlen(tag->key) : 0);
        if(!node)
            return panic("failed to add meta object");

        node->scope = tag->scope;
        node->type = tag->type | node->type;
        node->id = tag->id;

        if(tag->type & (meta_list | meta_map))
            root = node;

        tag++;
    }

    return 0;
}
