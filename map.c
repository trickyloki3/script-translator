#include "map.h"

#define is_nil(x)           ((x) == NULL)
#define is_root(x)          ((x)->parent == NULL)
#define is_left_child(x)    ((x)->parent->left == (x))
#define is_right_child(x)   ((x)->parent->right == (x))
#define is_black(x)         ((x) == NULL || (x)->color == black)
#define is_red(x)           ((x) != NULL && (x)->color == red)

int map_node_create(struct map *, void *, void *, struct map_node **);
void map_node_destroy(struct map *, struct map_node *);
static inline void map_node_attach(struct map_node *, struct map_node *);
static inline void map_node_detach(struct map_node *);

static inline void right_rotate(struct map *, struct map_node *);
static inline void left_rotate(struct map *, struct map_node *);
static inline void change_parent(struct map *, struct map_node *, struct map_node *);

static inline void map_insert_node(struct map *, struct map_node *);
static inline void map_delete_node(struct map *, struct map_node *);
static inline struct map_node * map_search_node(struct map *, void *);

int map_node_create(struct map * map, void * key, void * value, struct map_node ** result) {
    int status = 0;
    struct map_node * node;

    node = map->pool ? pool_get(map->pool) : malloc(sizeof(*node));
    if(!node) {
        status = panic("out of memory");
    } else {
        node->key = key;
        node->value = value;
        node->color = red;
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->next = node;
        node->prev = node;
        *result = node;
    }

    return status;
}

void map_node_destroy(struct map * map, struct map_node * node) {
    if(map->pool) {
        pool_put(map->pool, node);
    } else {
        free(node);
    }

}

static inline void map_node_attach(struct map_node * x, struct map_node * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

static inline void map_node_detach(struct map_node * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
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
            map_node_attach(x, p);
        } else {
            p->right = x;
            map_node_attach(p, x);
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
    map_node_detach(x);
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
        status = panic("compare is zero");
    } else if(pool && pool->size != sizeof(struct map_node)) {
        status = panic("pool is invalid");
    } else {
        map->compare = compare;
        map->pool = pool;
        map->root = NULL;
        map->iter = NULL;
    }

    return status;
}

void map_destroy(struct map * map) {
    struct map_node * node;

    if(map->root) {
        while(map->root != map->root->next) {
            node = map->root->next;
            map_node_detach(node);
            map_node_destroy(map, node);
        }
        map_node_destroy(map, map->root);
        map->root = NULL;
    }
    map->iter = NULL;
}

void map_clear(struct map * map) {
    map_destroy(map);
}

int map_copy(struct map * result, struct map * map) {
    int status = 0;
    struct map_node * iter;
    struct map_node * node;

    if(map_create(result, map->compare, map->pool)) {
        status = panic("failed to create map object");
    } else {
        if(map->root) {
            iter = map->root;
            do {
                if(map_node_create(result, iter->key, iter->value, &node)) {
                    status = panic("failed to create node object");
                } else {
                    map_insert_node(result, node);
                }
                iter = iter->next;
            } while(iter != map->root && !status);
        }

        if(status)
            map_destroy(result);
    }

    return status;
}

int map_insert(struct map * map, void * key, void * value) {
    int status = 0;
    struct map_node * node;

    node = map_search_node(map, key);
    if(node) {
        node->key = key;
        node->value = value;
    } else if(map_node_create(map, key, value, &node)) {
        status = panic("failed to create node object");
    } else {
        map_insert_node(map, node);
    }

    return status;
}

int map_delete(struct map * map, void * key) {
    int status = 0;
    struct map_node * node;

    node = map_search_node(map, key);
    if(!node) {
        status = panic("key does not exist in map object");
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

struct map_pair map_start(struct map * map) {
    map->iter = map->root;
    return map_next(map);
}

struct map_pair map_next(struct map * map) {
    struct map_pair pair = { NULL, NULL };

    if(map->iter) {
        pair.key = map->iter->key;
        pair.value = map->iter->value;
        map->iter = map->iter->next == map->root ? NULL : map->iter->next;
    }

    return pair;
}
