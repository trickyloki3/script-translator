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

struct logic_node * logic_and_and(struct logic *, struct logic_node *, struct logic_node *);
struct logic_node * logic_and_and_or(struct logic *, struct logic_node *, struct logic_node *);
struct logic_node * logic_or_and(struct logic *, struct logic_node *, struct logic_node *);
struct logic_node * logic_or_and_or(struct logic *, struct logic_node *, struct logic_node *);
int logic_not(struct logic *, struct logic_node *);
struct logic_node * logic_and_or_and_or(struct logic *, struct logic_node *, struct logic_node *);

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
        if(logic_and_cond(logic, node, type, data)) {
            status = panic("failed to and cond logic object");
        } else {
            logic_and_insert(logic, root, node);
        }
        if(status)
            logic_node_destroy(logic, node);
    }

    return status;
}

int logic_and_or_cond(struct logic * logic, struct logic_node * root, enum logic_type type, void * data) {
    int status = 0;
    struct logic_node * iter;

    iter = root->root;
    while(iter && !status) {
        if(logic_and_cond(logic, iter, type, data)) {
            status = panic("failed to and cond logic object");
        } else {
            iter = iter->next;
        }
    }

    return status;
}

struct logic_node * logic_and_and(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    int status = 0;

    struct logic_node * leaf;
    struct logic_node * iter;

    leaf = logic_node_copy(logic, root);
    if(!leaf) {
        status = panic("failed to copy logic node object");
    } else {
        iter = node->root;
        while(iter && !status) {
            if(logic_and_cond(logic, leaf, iter->type, iter->data)) {
                status = panic("failed to and cond logic object");
            } else {
                iter = iter->next;
            }
        }
        if(status)
            logic_node_destroy(logic, leaf);
    }

    return status ? NULL : leaf;
}

struct logic_node * logic_and_and_or(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    int status = 0;

    struct logic_node * leaf;
    struct logic_node * iter;

    leaf = logic_node_copy(logic, node);
    if(!leaf) {
        status = panic("failed to copy logic node object");
    } else {
        iter = root->root;
        while(iter && !status) {
            if(logic_and_or_cond(logic, leaf, iter->type, iter->data)) {
                status = panic("failed to and cond logic object");
            } else {
                iter = iter->next;
            }
        }
        if(status)
            logic_node_destroy(logic, leaf);
    }

    return status ? NULL : leaf;
}

struct logic_node * logic_or_and(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    int status = 0;

    struct logic_node * leaf;
    struct logic_node * copy;

    leaf = logic_node_copy(logic, root);
    if(!leaf) {
        status = panic("failed to copy logic node object");
    } else {
        copy = logic_node_copy(logic, node);
        if(!copy) {
            status = panic("failed to copy logic node object");
        } else {
            logic_and_insert(logic, leaf, copy);
        }
        if(status)
            logic_node_destroy(logic, leaf);
    }

    return status ? NULL : leaf;
}

struct logic_node * logic_or_and_or(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    int status = 0;

    struct logic_node * leaf;
    struct logic_node * iter;
    struct logic_node * copy;

    leaf = logic_node_copy(logic, root);
    if(!leaf) {
        status = panic("failed to copy logic node object");
    } else {
        iter = node->root;
        while(iter && !status) {
            copy = logic_node_copy(logic, iter);
            if(!copy) {
                status = panic("failed to copy logic node object");
            } else {
                logic_and_insert(logic, leaf, copy);
            }
            iter = iter->next;
        }
        if(status)
            logic_node_destroy(logic, leaf);
    }

    return status ? NULL : leaf;
}

int logic_not(struct logic * logic, struct logic_node * root) {
    int status = 0;

    struct logic_node * iter;

    if(root->type == cond) {
        if(logic_push(logic, not_cond, root->data))
            status = panic("failed to push logic object");
    } else if(root->type == not_cond) {
        if(logic_push(logic, cond, root->data))
            status = panic("failed to push logic object");
    } else if(root->type == and) {
        if(logic_push(logic, or, NULL)) {
            status = panic("failed to push logic object");
        } else {
            iter = root->root;
            while(iter && !status) {
                if(logic_not(logic, iter)) {
                    status = panic("failed to not logic object");
                } else {
                    iter = iter->next;
                }
            }
            if(!status && logic_pop(logic))
                status = panic("failed to pop logic object");
        }
    } else {
        status = panic("invalid type - %d", root->type);
    }

    return status;
}

struct logic_node * logic_and_or_and_or(struct logic * logic, struct logic_node * root, struct logic_node * node) {
    int status = 0;

    struct logic_node * leaf;
    struct logic_node * prev;
    struct logic_node * iter;
    struct logic_node * copy;

    leaf = logic_node_create(logic, and_or, NULL);
    if(!leaf) {
        status = panic("failed to create logic node object");
    } else {
        prev = root->root;
        while(prev && !status) {
            iter = node->root;
            while(iter && !status) {
                copy = logic_and_and(logic, prev, iter);
                if(!copy) {
                    status = panic("failed to and and logic object");
                } else {
                    logic_and_insert(logic, leaf, copy);
                }
                iter = iter->next;
            }
            prev = prev->next;
        }

        if(status)
            logic_node_destroy(logic, leaf);
    }

    return status ? NULL : leaf;
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
        logic->root = NULL;
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

    struct logic_node * prev;
    struct logic_node * iter;
    struct logic_node * copy;

    if(logic_create(result, logic->pool)) {
        status = panic("failed to create logic object");
    } else {
        prev = NULL;
        iter = logic->root;
        while(iter && !status) {
            copy = logic_node_copy(result, iter);
            if(!copy) {
                status = panic("failed to copy logic node object");
            } else {
                if(prev) {
                    prev->next = copy;
                } else {
                    result->root = copy;
                }
                prev = copy;
            }
            iter = iter->next;
        }

        if(status)
            logic_destroy(result);
    }

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
    struct logic_node * leaf;

    node = logic_pop_node(logic);
    if(!node) {
        status = panic("failed to pop logic node object");
    } else {
        if(node->root) {
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
                        leaf = logic_and_and(logic, root, node);
                        if(!leaf) {
                            status = panic("failed to and and logic object");
                        } else {
                            logic_push_node(logic, leaf);
                        }
                    } else if(node->type == and_or) {
                        leaf = logic_and_and_or(logic, root, node);
                        if(!leaf) {
                            status = panic("failed to and and or logic object");
                        } else {
                            logic_push_node(logic, leaf);
                        }
                    } else {
                        status = panic("invalid logic type");
                    }
                } else if(root->type == or) {
                    if(node->type == and) {
                        leaf = logic_or_and(logic, root, node);
                        if(!leaf) {
                            status = panic("failed to or and logic object");
                        } else {
                            logic_push_node(logic, leaf);
                        }
                    } else if(node->type == and_or) {
                        leaf = logic_or_and_or(logic, root, node);
                        if(!leaf) {
                            status = panic("failed to or and or logic object");
                        } else {
                            logic_push_node(logic, leaf);
                        }
                    } else {
                        status = panic("invalid logic type");
                    }
                } else if(root->type == not) {
                    if(node->type == and) {
                        if(logic_push(logic, or, NULL)) {
                            status = panic("failed to push logic object");
                        } else {
                            leaf = node->root;
                            while(leaf && !status) {
                                if(logic_not(logic, leaf)) {
                                    status = panic("failed to not logic object");
                                } else {
                                    leaf = leaf->next;
                                }
                            }
                        }
                    } else if(node->type == and_or) {
                        if(logic_push(logic, and, NULL)) {
                            status = panic("failed to push logic object");
                        } else {
                            leaf = node->root;
                            while(leaf && !status) {
                                if(logic_not(logic, leaf)) {
                                    status = panic("failed to not logic object");
                                } else {
                                    leaf = leaf->next;
                                }
                            }
                        }
                    } else {
                        status = panic("invalid logic type");
                    }
                } else if(root->type == and_or) {
                    if(node->type == and) {
                        leaf = root->root ? logic_and_and_or(logic, node, root) : logic_or_and(logic, root, node);
                        if(!leaf) {
                            status = panic("failed to and and or logic object");
                        } else {
                            logic_push_node(logic, leaf);
                        }
                    } else if(node->type == and_or) {
                        leaf = root->root ? logic_and_or_and_or(logic, root, node) : logic_or_and_or(logic, root, node);
                        if(!leaf) {
                            status = panic("failed to and or and or logic object");
                        } else {
                            logic_push_node(logic, leaf);
                        }
                    } else {
                        status = panic("invalid logic type");
                    }
                } else {
                    status = panic("invalid logic type");
                }
                logic_node_destroy(logic, root);
            }
        }
        logic_node_destroy(logic, node);
    }

    return status;
}

void logic_print(struct logic * logic) {
    if(logic->root)
        logic_node_print(logic->root, 0);
}
