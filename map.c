#include "map.h"

#define is_nil(x)           ((x) == NULL)
#define is_root(x)          ((x)->parent == NULL)
#define is_left_child(x)    ((x)->parent->left == (x))
#define is_right_child(x)   ((x)->parent->right == (x))
#define is_black(x)         ((x) == NULL || (x)->color == black)
#define is_red(x)           ((x) != NULL && (x)->color == red)

static inline struct map_node * map_node_create(struct map *, void *, void *);
static inline void map_node_destroy(struct map *, struct map_node *);

static inline void right_rotate(struct map *, struct map_node *);
static inline void left_rotate(struct map *, struct map_node *);
static inline void change_parent(struct map *, struct map_node *, struct map_node *);

static inline void map_insert_node(struct map *, struct map_node *);
static inline void map_delete_node(struct map *, struct map_node *);
static inline struct map_node * map_search_node(struct map *, void *);

static inline struct map_node * map_node_create(struct map * map, void * key, void * value) {
    struct map_node * node;

    node = pool_get(map->pool);
    if(node) {
        node->key = key;
        node->value = value;
        node->color = red;
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
    }

    return node;
}

static inline void map_node_destroy(struct map * map, struct map_node * node) {
    pool_put(map->pool, node);
}

static inline void right_rotate(struct map * map, struct map_node * x) {
    struct map_node * y;

    y = x->left;
    x->left = y->right;
    if(x->left)
        x->left->parent = x;

    y->parent = x->parent;
    if(is_root(x))
        map->root = y;
    else if(is_left_child(x))
        y->parent->left = y;
    else
        y->parent->right = y;

    x->parent = y;
    y->right = x;
}

static inline void left_rotate(struct map * map, struct map_node * x) {
    struct map_node * y;

    y = x->right;
    x->right = y->left;
    if(x->right)
        x->right->parent = x;

    y->parent = x->parent;
    if(is_root(x))
        map->root = y;
    else if(is_left_child(x))
        y->parent->left = y;
    else
        y->parent->right = y;

    x->parent = y;
    y->left = x;
}

static inline void change_parent(struct map * map, struct map_node * x, struct map_node * y) {
    if(is_nil(x->parent))
        map->root = y;
    else if(is_left_child(x))
        x->parent->left = y;
    else
        x->parent->right = y;

    if(y)
        y->parent = x->parent;
}

static inline void map_insert_node(struct map * map, struct map_node * x) {
    struct map_node * p;
    struct map_node * i;
    struct map_node * s;

    p = NULL;
    i = map->root;
    while(i) {
        p = i;
        i = 0 > map->compare(x->key, i->key) ? i->left : i->right;
    }

    x->parent = p;
    if(is_nil(p)) {
        map->root = x;
    } else {
        if(0 > map->compare(x->key, p->key)) {
            p->left = x;
        } else {
            p->right = x;
        }
    }

    while(is_red(p)) {
        if(is_left_child(p)) {
            s = p->parent->right;
            if(is_red(s)) {
                p->color = black;
                s->color = black;
                p->parent->color = red;
                x = p->parent;
                p = x->parent;
            } else {
                if(is_right_child(x)) {
                    x = x->parent;
                    left_rotate(map, x);
                    p = x->parent;
                }
                p->color = black;
                p->parent->color = red;
                right_rotate(map, p->parent);
            }
        } else {
            s = p->parent->left;
            if(is_red(s)) {
                p->color = black;
                s->color = black;
                p->parent->color = red;
                x = p->parent;
                p = x->parent;
            } else {
                if(is_left_child(x)) {
                    x = x->parent;
                    right_rotate(map, x);
                    p = x->parent;
                }
                p->color = black;
                p->parent->color = red;
                left_rotate(map, p->parent);
            }
        }
    }

    map->root->color = black;
}

static inline void map_delete_node(struct map * map, struct map_node * x) {
    struct map_node * y;
    struct map_node * p;
    enum map_color c;

    struct map_node * z;
    struct map_node * s;

    if(is_nil(x->left)) {
        y = x->right;
        p = x->parent;
        c = x->color;
        change_parent(map, x, x->right);
    } else if(is_nil(x->right)) {
        y = x->left;
        p = x->parent;
        c = x->color;
        change_parent(map, x, x->left);
    } else {
        z = x->right;
        while(z->left)
            z = z->left;

        y = z->right;
        p = z->parent == x ? z : z->parent;
        c = z->color;

        if(z->parent != x) {
            change_parent(map, z, z->right);
            z->right = x->right;
            z->right->parent = z;
        }
        z->left = x->left;
        z->left->parent = z;
        z->color = x->color;
        change_parent(map, x, z);
    }

    if(c == black) {
        while(p && is_black(y)) {
            if(y == p->left) {
                s = p->right;

                if(is_red(s)) {
                    s->color = black;
                    p->color = red;
                    left_rotate(map, p);
                    s = p->right;
                }

                if(is_black(s->left) && is_black(s->right)) {
                    s->color = red;
                    y = p;
                    p = y->parent;
                } else {
                    if(is_black(s->right)) {
                        s->color = red;
                        s->left->color = black;
                        right_rotate(map, s);
                        s = p->right;
                    }
                    s->color = p->color;
                    s->right->color = black;
                    p->color = black;
                    left_rotate(map, p);
                    y = map->root;
                    p = y->parent;
                }
            } else {
                s = p->left;

                if(is_red(s)) {
                    s->color = black;
                    p->color = red;
                    right_rotate(map, p);
                    s = p->left;
                }

                if(is_black(s->left) && is_black(s->right)) {
                    s->color = red;
                    y = p;
                    p = y->parent;
                } else {
                    if(is_black(s->left)) {
                        s->color = red;
                        s->right->color = black;
                        left_rotate(map, s);
                        s = p->left;
                    }
                    s->color = p->color;
                    s->left->color = black;
                    p->color = black;
                    right_rotate(map, p);
                    y = map->root;
                    p = y->parent;
                }
            }
        }
        if(y)
            y->color = black;
    }

    x->left = NULL;
    x->right = NULL;
}

static inline struct map_node * map_search_node(struct map * map, void * key) {
    int status;
    struct map_node * i;

    i = map->root;
    while(i) {
        status = map->compare(key, i->key);
        if(0 == status) {
            break;
        } else if(0 > status) {
            i = i->left;
        } else {
            i = i->right;
        }
    }

    return i;
}

int map_create(struct map * map, map_compare_cb compare, struct pool * pool) {
    int status = 0;

    if(!compare) {
        status = panic("invalid compare");
    } else if(!pool || pool->size < sizeof(struct map_node)) {
        status = panic("invalid pool");
    } else {
        map->compare = compare;
        map->pool = pool;
        map->root = NULL;
    }

    return status;
}

void map_destroy(struct map * map) {
    map_clear(map);
}

void map_clear(struct map * map) {
    struct map_node * node;
    struct map_node * temp;

    node = map->root;
    while(node) {
        while(node->left)
            node = node->left;
        if(node->right) {
            node = node->right;
        } else {
            temp = node;
            node = node->parent;
            if(node) {
                if(temp == node->left) {
                    node->left = NULL;
                } else {
                    node->right = NULL;
                }
            }
            map_node_destroy(map, temp);
        }
    }
    map->root = NULL;
}

int map_insert(struct map * map, void * key, void * value) {
    int status = 0;
    struct map_node * node;

    node = map_search_node(map, key);
    if(node) {
        node->key = key;
        node->value = value;
    } else {
        node = map_node_create(map, key, value);
        if(!node) {
            status = panic("failed to create node object");
        } else {
            map_insert_node(map, node);
        }
    }

    return status;
}

int map_delete(struct map * map, void * key) {
    int status = 0;
    struct map_node * node;

    node = map_search_node(map, key);
    if(!node) {
        status = panic("invalid key");
    } else {
        map_delete_node(map, node);
        map_node_destroy(map, node);
    }

    return status;
}

void * map_search(struct map * map, void * key) {
    struct map_node * node = map_search_node(map, key);
    return node ? node->value : NULL;
}
