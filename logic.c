#include "logic.h"

struct logic_node * logic_node_create(struct logic *, enum logic_type, void *);
void logic_node_destroy(struct logic *, struct logic_node *);
struct logic_node * logic_node_copy(struct logic *, struct logic_node *);
void logic_node_print(struct logic_node *, int);

void logic_cond_insert(struct logic *, struct logic_node *, struct logic_node *);
int logic_and_subset(struct logic_node *, struct logic_node *);
void logic_and_insert(struct logic *, struct logic_node *, struct logic_node *);

int logic_and_cond(struct logic *, struct logic_node *, enum logic_type, void *);
int logic_or_cond(struct logic *, struct logic_node *, enum logic_type, void *);
int logic_and_or_cond(struct logic *, struct logic_node *, enum logic_type, void *);

void logic_and(struct logic *, struct logic_node *, struct logic_node *);
void logic_or(struct logic *, struct logic_node *, struct logic_node *);
struct logic_node * logic_not_and(struct logic *, struct logic_node *);
struct logic_node * logic_not_or(struct logic *, struct logic_node *);
int logic_and_or_and(struct logic *, struct logic_node *, struct logic_node *);
struct logic_node * logic_and_or_or(struct logic *, struct logic_node *, struct logic_node *);

void logic_push_node(struct logic *, struct logic_node *);
struct logic_node * logic_pop_node(struct logic *);

struct logic_node * logic_node_create(struct logic * logic, enum logic_type type, void * data) {
    struct logic_node * node;

    node = pool_get(logic->pool);
    if(node) {
        node->type = type;
        node->data = data;
        node->root = NULL;
        node->next = NULL;
    }

    return node;
}

void logic_node_destroy(struct logic * logic, struct logic_node * root) {
    struct logic_node * node;

    while(root->root) {
        node = root->root;
        root->root = root->root->next;
        logic_node_destroy(logic, node);
    }

    pool_put(logic->pool, root);
}

struct logic_node * logic_node_copy(struct logic * logic, struct logic_node * root) {
    int status = 0;

    struct logic_node * node;
    struct logic_node * prev;
    struct logic_node * iter;
    struct logic_node * copy;

    node = logic_node_create(logic, root->type, root->data);
    if(!node) {
        status = panic("failed to create logic node object");
    } else {
        prev = NULL;
        iter = root->root;
        while(iter && !status) {
            copy = logic_node_copy(logic, iter);
            if(!copy) {
                status = panic("failed to copy logic node object");
            } else {
                if(prev) {
                    prev->next = copy;
                } else {
                    node->root = copy;
                }
                prev = copy;
            }
            iter = iter->next;
        }
        if(status)
            logic_node_destroy(logic, node);
    }

    return status ? NULL : node;
}

void logic_node_print(struct logic_node * root, int indent) {
    int i;

    struct logic_node * iter;

    for(i = 0; i < indent; i++)
        fputs("    ", stdout);

    if(root->type == cond) {
        fprintf(stdout, "[cond:%p]\n", root->data);
    } else if(root->type == not_cond) {
        fprintf(stdout, "[not_cond:%p]\n", root->data);
    } else if(root->type == and) {
        fprintf(stdout, "[and]\n");
    } else if(root->type == or) {
        fprintf(stdout, "[or]\n");
    } else if(root->type == not) {
        fprintf(stdout, "[not]\n");
    } else if(root->type == and_or) {
        fprintf(stdout, "[and_or]\n");
    }

    iter = root->root;
    while(iter) {
        logic_node_print(iter, indent + 1);
        iter = iter->next;
    }
}

void logic_cond_insert(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    struct logic_node * prev;
    struct logic_node * iter;

    prev = NULL;
    iter = root->root;
    while(iter && iter->data <= node->data) {
        prev = iter;
        iter = iter->next;
    }

    if(!prev) {
        node->next = root->root;
        root->root = node;
    } else if(prev->data == node->data) {
        logic_node_destroy(logic, node);
    } else {
        node->next = prev->next;
        prev->next = node;
    }
}

int logic_and_subset(struct logic_node * root, struct logic_node * node) {
    struct logic_node * prev;
    struct logic_node * iter;

    prev = root->root;
    while(prev && prev->data < node->data)
        prev = prev->next;

    iter = node->root;
    while(prev && iter && prev->data == iter->data && prev->type == iter->type) {
        prev = prev->next;
        iter = iter->next;
    }

    return iter ? 1 : 0;
}

void logic_and_insert(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    struct logic_node * prev;
    struct logic_node * iter;

    iter = root->root;
    while(iter && logic_and_subset(node, iter))
        iter = iter->next;

    if(iter) {
        logic_node_destroy(logic, node);
    } else {
        prev = node;
        while(root->root) {
            iter = root->root;
            root->root = root->root->next;
            if(logic_and_subset(iter, node)) {
                prev = prev->next = iter;
            } else {
                logic_node_destroy(logic, iter);
            }
        }
        prev->next = NULL;
        root->root = node;
    }
}

int logic_and_cond(struct logic * logic, struct logic_node * root, enum logic_type type, void * data) {
    int status = 0;
    struct logic_node * node;

    node = logic_node_create(logic, type, data);
    if(!node) {
        status = panic("failed to create logic node object");
    } else {
        logic_cond_insert(logic, root, node);
    }

    return status;
}

int logic_or_cond(struct logic * logic, struct logic_node * root, enum logic_type type, void * data) {
    int status = 0;
    struct logic_node * node;

    node = logic_node_create(logic, and, NULL);
    if(!node) {
        status = panic("failed to create logic node object");
    } else {
        if(logic_and_cond(logic, node, type, data))
            status = panic("failed to and cond logic object");
        if(status) {
            logic_node_destroy(logic, node);
        } else {
            logic_and_insert(logic, root, node);
        }
    }

    return status;
}

int logic_and_or_cond(struct logic * logic, struct logic_node * root, enum logic_type type, void * data) {
    int status = 0;
    struct logic_node * iter;

    iter = root->root;
    while(iter && !status) {
        if(logic_and_cond(logic, iter, type, data))
            status = panic("failed to and cond logic object");
        iter = iter->next;
    }

    return status;
}

void logic_and(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    struct logic_node * iter;

    while(node->root) {
        iter = node->root;
        node->root = node->root->next;
        logic_cond_insert(logic, root, iter);
    }

    logic_node_destroy(logic, node);
}

void logic_or(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    struct logic_node * iter;

    while(node->root) {
        iter = node->root;
        node->root = node->root->next;
        logic_and_insert(logic, root, iter);
    }

    logic_node_destroy(logic, node);
}

struct logic_node * logic_not_and(struct logic * logic, struct logic_node * root) {
    int status = 0;
    struct logic_node * node;
    struct logic_node * iter;

    node = logic_node_create(logic, or, NULL);
    if(!node) {
        status = panic("failed to create logic node object");
    } else {
        iter = root->root;
        while(iter && !status) {
            if(iter->type == cond) {
                if(logic_or_cond(logic, node, not_cond, iter->data))
                    status = panic("failed to or cond logic object");
            } else if(iter->type == not_cond) {
                if(logic_or_cond(logic, node, cond, iter->data))
                    status = panic("failed to or cond logic object");
            } else {
                status = panic("invalid logic type");
            }
            iter = iter->next;
        }
        if(status)
            logic_node_destroy(logic, node);
    }

    return status ? NULL : node;
}

struct logic_node * logic_not_or(struct logic * logic, struct logic_node * root) {
    int status = 0;
    struct logic_node * node;
    struct logic_node * iter;
    struct logic_node * copy;
    struct logic_node * temp;

    node = logic_not_and(logic, root->root);
    if(!node) {
        status = panic("failed to not and logic object");
    } else {
        iter = root->root->next;
        while(iter && !status) {
            copy = logic_not_and(logic, iter);
            if(!copy) {
                status = panic("failed to not and logic object");
            } else {
                temp = logic_and_or_or(logic, node, copy);
                if(!temp) {
                    status = panic("failed to and or or logic object");
                } else {
                    logic_node_destroy(logic, node);
                    node = temp;
                }
                logic_node_destroy(logic, copy);
            }
            iter = iter->next;
        }
        if(status)
            logic_node_destroy(logic, node);
    }

    return status ? NULL : node;
}

int logic_and_or_and(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    int status = 0;
    struct logic_node * iter;

    iter = node->root;
    while(iter && !status) {
        if(logic_and_or_cond(logic, root, iter->type, iter->data))
            status = panic("failed to and or cond logic object");
        iter = iter->next;
    }

    return status;
}

struct logic_node * logic_and_or_or(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    int status = 0;
    struct logic_node * temp;
    struct logic_node * iter;
    struct logic_node * copy;

    temp = logic_node_create(logic, and_or, NULL);
    if(!temp) {
        status = panic("failed to create logic node object");
    } else {
        iter = root->root;
        while(iter && !status) {
            copy = logic_node_copy(logic, node);
            if(!copy) {
                status = panic("failed to copy logic node object");
            } else {
                if(logic_and_or_and(logic, copy, iter))
                    status = panic("failed to and or logic object");
                if(status) {
                    logic_node_destroy(logic, copy);
                } else {
                    logic_or(logic, temp, copy);
                }
            }
            iter = iter->next;
        }
        if(status)
            logic_node_destroy(logic, temp);
    }

    return status ? NULL : temp;
}

void logic_push_node(struct logic * logic, struct logic_node * node) {
    node->next = logic->root;
    logic->root = node;
}

struct logic_node * logic_pop_node(struct logic * logic) {
    struct logic_node * node = NULL;

    if(logic->root) {
        node = logic->root;
        logic->root = logic->root->next;
    }

    return node;
}

int logic_create(struct logic * logic, struct pool * pool) {
    int status = 0;

    if(!pool || pool->size < sizeof(struct logic_node)) {
        status = panic("invalid pool");
    } else {
        logic->pool = pool;
        logic->root = logic_node_create(logic, or, NULL);
        if(!logic->root)
            status = panic("failed to create logic node object");
    }

    return status;
}

void logic_destroy(struct logic * logic) {
    struct logic_node * node;

    while(logic->root) {
        node = logic->root;
        logic->root = logic->root->next;
        logic_node_destroy(logic, node);
    }
}

int logic_copy(struct logic * result, struct logic * logic) {
    int status = 0;

    result->pool = logic->pool;
    result->root = logic_node_copy(result, logic->root);
    if(!result->root)
        status = panic("failed to copy logic node object");

    return status;
}

int logic_push(struct logic * logic, enum logic_type type, void * data) {
    int status = 0;
    struct logic_node * node;

    if(type == cond || type == not_cond) {
        node = logic->root;
        if(!node) {
            status = panic("invalid logic node object");
        } else if(node->type == and) {
            if(logic_and_cond(logic, node, type, data))
                status = panic("failed to and logic object");
        } else if(node->type == or) {
            if(logic_or_cond(logic, node, type, data))
                status = panic("failed to or logic object");
        } else if(node->type == not) {
            if(logic_and_cond(logic, node, type == cond ? not_cond : cond, data))
                status = panic("failed to and logic object");
        } else if(node->type == and_or) {
            if(logic_and_or_cond(logic, node, type, data))
                status = panic("failed to and or logic object");
        } else {
            status = panic("invalid logic type");
        }
    } else if(type == and || type == or || type == not) {
        node = logic_node_create(logic, type, data);
        if(!node) {
            status = panic("failed to create logic node object");
        } else {
            logic_push_node(logic, node);
        }
    } else {
        status = panic("invalid logic type");
    }

    return status;
}

int logic_pop(struct logic * logic) {
    int status = 0;
    struct logic_node * node;
    struct logic_node * root;
    struct logic_node * temp;

    node = logic_pop_node(logic);
    if(!node) {
        status = panic("failed to pop logic node object");
    } else if(!node->root) {
        logic_node_destroy(logic, node);
    } else {
        root = logic_pop_node(logic);
        if(!root) {
            status = panic("failed to pop logic node object");
        } else {
            if(node->type == not) {
                node->type = and;
            } else if(node->type == or) {
                node->type = and_or;
            }

            if(root->type == and) {
                if(node->type == and) {
                    logic_and(logic, root, node);
                    logic_push_node(logic, root);
                } else if(node->type == and_or) {
                    if(logic_and_or_and(logic, node, root)) {
                        status = panic("failed to and or logic object");
                    } else {
                        logic_node_destroy(logic, root);
                        logic_push_node(logic, node);
                    }
                } else {
                    status = panic("invalid logic type");
                }
            } else if(root->type == or) {
                if(node->type == and) {
                    logic_and_insert(logic, root, node);
                    logic_push_node(logic, root);
                } else if(node->type == and_or) {
                    logic_or(logic, root, node);
                    logic_push_node(logic, root);
                } else {
                    status = panic("invalid logic type");
                }
            } else if(root->type == not) {
                if(node->type == and) {
                    temp = logic_not_and(logic, node);
                    if(!temp) {
                        status = panic("failed to not and logic object");
                    } else {
                        logic_node_destroy(logic, node);
                        logic_node_destroy(logic, root);
                        logic_push_node(logic, temp);
                    }
                } else if(node->type == and_or) {
                    temp = logic_not_or(logic, node);
                    if(!temp) {
                        status = panic("failed to not and logic object");
                    } else {
                        logic_node_destroy(logic, node);
                        logic_node_destroy(logic, root);
                        logic_push_node(logic, temp);
                    }
                } else {
                    status = panic("invalid logic type");
                }
            } else if(root->type == and_or) {
                if(node->type == and) {
                    if(logic_and_or_and(logic, root, node)) {
                        status = panic("failed to and or logic object");
                    } else {
                        logic_node_destroy(logic, node);
                        logic_push_node(logic, root);
                    }
                } else if(node->type == and_or) {
                    temp = logic_and_or_or(logic, root, node);
                    if(!temp) {
                        status = panic("failed to and or product logic object");
                    } else {
                        logic_node_destroy(logic, node);
                        logic_node_destroy(logic, root);
                        logic_push_node(logic, temp);
                    }
                } else {
                    status = panic("invalid logic type");
                }
            } else {
                status = panic("invalid logic type");
            }
            if(status)
                logic_node_destroy(logic, root);
        }
        if(status)
            logic_node_destroy(logic, node);
    }

    return status;
}

void logic_print(struct logic * logic) {
    if(logic->root)
        logic_node_print(logic->root, 0);
}
