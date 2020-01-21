#include "logic.h"

struct logic_node * logic_node_create(struct logic *, enum logic_type, void *);
void logic_node_destroy(struct logic *, struct logic_node *);
void logic_node_clear(struct logic *, struct logic_node *);
struct logic_node * logic_node_copy(struct logic *, struct logic_node *);
void logic_node_push(struct logic_node *, struct logic_node *);
struct logic_node * logic_node_pop(struct logic_node *);
struct logic_node * logic_node_top(struct logic_node *);
void logic_node_print(struct logic_node *, int);

int logic_cond(struct logic *, struct logic_node *, enum logic_type, void *);
int logic_and(struct logic *, struct logic_node *, struct logic_node *);
int logic_or(struct logic *, struct logic_node *, struct logic_node *);

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

void logic_node_destroy(struct logic * logic, struct logic_node * node) {
    logic_node_clear(logic, node);
    pool_put(logic->pool, node);
}

void logic_node_clear(struct logic * logic, struct logic_node * list) {
    struct logic_node * node;

    node = logic_node_pop(list);
    while(node) {
        logic_node_destroy(logic, node);
        node = logic_node_pop(list);
    }
}

struct logic_node * logic_node_copy(struct logic * logic, struct logic_node * list) {
    int status = 0;
    struct logic_node * node;
    struct logic_node * iter;
    struct logic_node * copy;

    node = logic_node_create(logic, list->type, list->data);
    if(!node) {
        status = panic("failed to create logic node object");
    } else {
        iter = list->root;
        while(iter && !status) {
            copy = logic_node_copy(logic, iter);
            if(!copy) {
                status = panic("failed to copy logic node object");
            } else {
                logic_node_push(node, copy);
            }
            iter = iter->next;
        }
        if(status)
            logic_node_destroy(logic, node);
    }

    return status ? NULL : node;
}

void logic_node_push(struct logic_node * list, struct logic_node * node) {
    node->next = list->root;
    list->root = node;
}

struct logic_node * logic_node_pop(struct logic_node * list) {
    struct logic_node * node = NULL;

    if(list->root) {
        node = list->root;
        list->root = list->root->next;
    }

    return node;
}

struct logic_node * logic_node_top(struct logic_node * list) {
    return list->root;
}

void logic_node_print(struct logic_node * node, int indent) {
    int i;
    struct logic_node * iter;

    for(i = 0; i < indent; i++)
        fputs("    ", stdout);

    if(node->type == cond) {
        fprintf(stdout, "[cond:%p]\n", node->data);
    } else if(node->type == not_cond) {
        fprintf(stdout, "[not_cond:%p]\n", node->data);
    } else {
        if(node->type == and) {
            fprintf(stdout, "[and]\n");
        } else if(node->type == or) {
            fprintf(stdout, "[or]\n");
        } else if(node->type == not) {
            fprintf(stdout, "[not]\n");
        } else if(node->type == and_or) {
            fprintf(stdout, "[and_or]\n");
        }
        iter = node->root;
        while(iter) {
            logic_node_print(iter, indent + 1);
            iter = iter->next;
        }
    }
}

int logic_create(struct logic * logic, struct pool * pool) {
    int status = 0;

    if(!pool || pool->size < sizeof(struct logic_node)) {
        status = panic("invalid pool");
    } else {
        logic->pool = pool;
        logic->root = logic_node_create(logic, or, NULL);
        if(!logic->root) {
            status = panic("failed to create logic node object");
        } else if(logic_push(logic, or, NULL)) {
            status = panic("failed to push logic object");
        }
    }

    return status;
}

void logic_destroy(struct logic * logic) {
    logic_node_destroy(logic, logic->root);
}

int logic_clear(struct logic * logic) {
    int status = 0;

    logic_node_clear(logic, logic->root);

    if(logic_push(logic, or, NULL))
        status = panic("failed to push logic object");

    return status;
}

int logic_copy(struct logic * result, struct logic * logic) {
    int status = 0;

    result->pool = logic->pool;
    result->root = logic_node_copy(result, logic->root);
    if(!result->root)
        status = panic("failed to copy logic node object");

    return status;
}

int logic_cond(struct logic * logic, struct logic_node * list, enum logic_type type, void * data) {
    int status = 0;
    struct logic_node * iter;

    if(type == cond || type == not_cond) {
        if(list->type == and || list->type == or) {
            iter = logic_node_create(logic, type, data);
            if(!iter) {
                status = panic("failed to create logic node object");
            } else {
                logic_node_push(list, iter);
            }
        } else if(list->type == not) {
            iter = logic_node_create(logic, type == cond ? not_cond : cond, data);
            if(!iter) {
                status = panic("failed to create logic node object");
            } else {
                logic_node_push(list, iter);
            }
        } else if(list->type == and_or) {
            iter = list->root;
            while(iter && !status) {
                if(logic_cond(logic, iter, type, data))
                    status = panic("failed to cond logic object");
                iter = iter->next;
            }
        } else {
            status = panic("invalid logic type");
        }
    } else {
        status = panic("invalid logic type");
    }

    return status;
}

int logic_and(struct logic * logic, struct logic_node * list, struct logic_node * node) {
    int status = 0;
    struct logic_node * iter;

    if(node->type == and || node->type == not) {
        iter = node->root;
        while(iter && !status) {
            if(logic_and(logic, list, iter))
                status = panic("failed to and logic object");
            iter = iter->next;
        }
    } else if(logic_cond(logic, list, node->type, node->data)) {
        status = panic("failed to cond logic object");
    }

    return status;
}

int logic_or(struct logic * logic, struct logic_node * list, struct logic_node * node) {
    int status = 0;
    struct logic_node * copy;

    if(node->type == and) {
        copy = logic_node_copy(logic, node);
        if(!copy) {
            status = panic("failed to copy logic node object");
        } else {
            logic_node_push(list, copy);
        }
    } else if(logic_cond(logic, list, node->type, node->data)) {
        status = panic("failed to cond logic object");
    }

    return status;
}

int logic_push(struct logic * logic, enum logic_type type, void * data) {
    int status = 0;
    struct logic_node * node;

    if(type == and || type == or || type == not) {
        node = logic_node_create(logic, type, data);
        if(!node) {
            status = panic("failed to create logic node object");
        } else {
            logic_node_push(logic->root, node);
        }
    } else {
        node = logic_node_top(logic->root);
        if(!node) {
            status = panic("invalid logic node object");
        } else if(logic_cond(logic, node, type, data)){
            status = panic("failed to cond logic object");
        }
    }

    return status;
}

int logic_pop(struct logic * logic) {
    int status = 0;
    struct logic_node * r;
    struct logic_node * l;
    struct logic_node * node;
    struct logic_node * iter;
    struct logic_node * copy;
    struct logic_node * x;
    struct logic_node * y;

    r = logic_node_pop(logic->root);
    if(!r) {
        status = panic("failed to pop logic node object");
    } else {
        l = logic_node_pop(logic->root);
        if(!l) {
            status = panic("failed to pop logic node object");
        } else {
            if(l->type == and) {
                if(r->type == and || r->type == not) {
                    copy = logic_node_copy(logic, l);
                    if(!copy) {
                        status = panic("failed to copy logic node object");
                    } else {
                        if(logic_and(logic, copy, r))
                            status = panic("failed to and logic object");
                        if(status) {
                            logic_node_destroy(logic, copy);
                        } else {
                            logic_node_push(logic->root, copy);
                        }
                    }
                } else if(r->type == or) {
                    node = logic_node_create(logic, and_or, NULL);
                    if(!node) {
                        status = panic("failed create logic node object");
                    } else {
                        iter = r->root;
                        while(iter && !status) {
                            copy = logic_node_copy(logic, l);
                            if(!copy) {
                                status = panic("failed to copy logic node object");
                            } else {
                                if(logic_and(logic, copy, iter))
                                    status = panic("failed to and logic object");
                                if(status) {
                                    logic_node_destroy(logic, copy);
                                } else {
                                    logic_node_push(node, copy);
                                }
                            }
                            iter = iter->next;
                        }
                        if(status) {
                            logic_node_destroy(logic, node);
                        } else {
                            logic_node_push(logic->root, node);
                        }
                    }
                } else if(r->type == and_or) {
                    copy = logic_node_copy(logic, r);
                    if(!copy) {
                        status = panic("failed to copy logic node object");
                    } else {
                        iter = copy->root;
                        while(iter && !status) {
                            if(logic_and(logic, iter, l))
                                status = panic("failed to and logic object");
                            iter = iter->next;
                        }
                        if(status) {
                            logic_node_destroy(logic, copy);
                        } else {
                            logic_node_push(logic->root, copy);
                        }
                    }
                } else {
                    status = panic("invalid logic type");
                }
            } else if(l->type == or) {
                if(r->type == and) {
                    copy = logic_node_copy(logic, l);
                    if(!copy) {
                        status = panic("failed to copy logic node object");
                    } else {
                        if(logic_or(logic, copy, r))
                            status = panic("failed to or logic object");
                        if(status) {
                            logic_node_destroy(logic, copy);
                        } else {
                            logic_node_push(logic->root, copy);
                        }
                    }
                } else if(r->type == or || r->type == not || r->type == and_or) {
                    copy = logic_node_copy(logic, l);
                    if(!copy) {
                        status = panic("failed to copy logic node object");
                    } else {
                        iter = r->root;
                        while(iter && !status) {
                            if(logic_or(logic, copy, iter))
                                status = panic("failed to or logic object");
                            iter = iter->next;
                        }
                        if(status) {
                            logic_node_destroy(logic, copy);
                        } else {
                            logic_node_push(logic->root, copy);
                        }
                    }
                } else {
                    status = panic("invalid logic type");
                }
            } else if(l->type == and_or) {
                if(r->type == and || r->type == not) {
                    copy = logic_node_copy(logic, l);
                    if(!copy) {
                        status = panic("failed to copy logic node object");
                    } else {
                        iter = copy->root;
                        while(iter && !status) {
                            if(logic_and(logic, iter, r))
                                status = panic("failed to and logic object");
                            iter = iter->next;
                        }
                        if(status) {
                            logic_node_destroy(logic, copy);
                        } else {
                            logic_node_push(logic->root, copy);
                        }
                    }
                } else if(r->type == or || r->type == and_or) {
                    node = logic_node_create(logic, and_or, NULL);
                    if(!node) {
                        status = panic("failed create logic node object");
                    } else {
                        x = r->root;
                        while(x && !status) {
                            y = l->root;
                            while(y && !status) {
                                copy = logic_node_copy(logic, y);
                                if(!copy) {
                                    status = panic("failed to copy logic node object");
                                } else {
                                    if(logic_and(logic, copy, x))
                                        status = panic("failed to and logic object");
                                    if(status) {
                                        logic_node_destroy(logic, copy);
                                    } else {
                                        logic_node_push(node, copy);
                                    }
                                }
                                y = y->next;
                            }
                            x = x->next;
                        }
                        if(status) {
                            logic_node_destroy(logic, node);
                        } else {
                            logic_node_push(logic->root, node);
                        }
                    }
                } else {
                    status = panic("invalid logic type");
                }
            } else {
                status = panic("invalid logic type");
            }
            logic_node_destroy(logic, l);
        }
        logic_node_destroy(logic, r);
    }

    return status;
}

void logic_print(struct logic * logic) {
    struct logic_node * node;

    node = logic_node_top(logic->root);
    if(node)
        logic_node_print(node, 0);
}
