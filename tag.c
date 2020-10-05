#include "tag.h"

struct tag_node * tag_node_create(struct tag * tag, struct tag_node * root,  char * key, size_t size) {
    struct tag_node * node;

    node = zone_get(&tag->zone, sizeof(*node) + (size + 1));
    if(!node)
        return NULL;

    if(map_create(&node->map, (map_compare_cb) strcmp, &tag->pool))
        return NULL;

    node->scope = 0;
    node->type = 0;
    node->id = 0;
    node->key = (char *) (node + 1);
    node->list = NULL;
    node->prev = root;
    node->tag = tag;

    memcpy(node->key, key, size);
    node->key[size] = '\0';

    return node;
}

void tag_node_destroy(struct tag_node * node) {
    struct map_kv kv;

    kv = map_start(&node->map);
    while(kv.value) {
        tag_node_destroy(kv.value);
        kv = map_next(&node->map);
    }

    if(node->list)
        tag_node_destroy(node->list);

    map_destroy(&node->map);
}

void tag_node_print(struct tag_node * node, int indent, char * key) {
    int i;
    struct map_kv kv;

    for(i = 0; i < indent; i++)
        fputs("  ", stdout);

    if(key)
        fprintf(stdout, "(%s): ", key);

    fprintf(stdout, "[%d", node->id);

    if(node->type & tag_map)
        fputs(", map", stdout);

    if(node->type & tag_list)
        fputs(", list", stdout);

    if(node->type & tag_string)
        fputs(", string", stdout);

    fputs("]\n", stdout);

    kv = map_start(&node->map);
    while(kv.key) {
        tag_node_print(kv.value, indent + 1, kv.key);
        kv = map_next(&node->map);
    }

    if(node->list)
        tag_node_print(node->list, indent + 1, NULL);
}

int tag_create(struct tag * tag, size_t size) {
    int status = 0;

    if(pool_create(&tag->pool, sizeof(struct map_node), size / sizeof(struct map_node))) {
        status = panic("failed to create pool object");
    } else {
        if(zone_create(&tag->zone, size)) {
            status = panic("failed to create zone object");
        } else {
            tag->root = NULL;
        }
        if(status)
            pool_destroy(&tag->pool);
    }

    return status;
}

void tag_destroy(struct tag * tag) {
    tag_clear(tag);
    zone_destroy(&tag->zone);
    pool_destroy(&tag->pool);
}

void tag_clear(struct tag * tag) {
    struct tag_node * node;

    if(tag->root) {
        node = tag->root;
        tag->root = NULL;
        tag_node_destroy(node);
    }

    zone_clear(&tag->zone);
}

void tag_print(struct tag * tag) {
    if(tag->root)
        tag_node_print(tag->root, 0, NULL);
}

int tag_load(struct tag * tag, struct tag_node * iter) {
    struct tag_node * root;
    struct tag_node * node;

    tag_clear(tag);

    tag->root = tag_node_create(tag, NULL, NULL, 0);
    if(!tag->root)
        return panic("failed to create tag node object");

    if(iter) {
        root = tag->root;

        while(iter->scope > 0) {
            while(root->scope >= iter->scope)
                root = root->prev;

            node = tag_add(root, iter->key, iter->key ? strlen(iter->key) : 0);
            if(!node)
                return panic("failed to add tag object");

            node->scope = iter->scope;
            node->type = iter->type | node->type;
            node->id = iter->id;

            if(iter->type & (tag_list | tag_map))
                root = node;

            iter++;
        }
    }

    return 0;
}

struct tag_node * tag_add(struct tag_node * root, char * key, size_t size) {
    struct tag_node * node;

    if(key) {
        node = map_search(&root->map, key);
        if(node)
            return node;

        node = tag_node_create(root->tag, root, key, size);
        if(node) {
            if(map_insert(&root->map, node->key, node) == 0)
                return node;
            tag_node_destroy(node);
        }
    } else {
        node = root->list;
        if(node)
            return node;

        node = tag_node_create(root->tag, root, NULL, 0);
        if(node)
            return root->list = node;
    }

    return NULL;
}
