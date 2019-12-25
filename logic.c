#include "logic.h"

int logic_var_create(struct logic_var *, void *, struct string *, struct range *, struct logic *);
void logic_var_destroy(struct logic_var *);
int logic_var_merge(struct logic_var *, enum logic_type, struct range *);

int logic_node_var_create(struct logic *, void *, struct string *, struct range *, struct logic_node **);
int logic_node_create(struct logic *, enum logic_type, struct logic_node **);
void logic_node_destroy(struct logic *, struct logic_node *);
int logic_node_copy(struct logic *, struct logic_node *, struct logic_node **);
struct logic_node * logic_node_search(struct logic_node *, struct string *);
void logic_node_print(struct logic_node *, int);

int logic_add_one(struct logic *, struct logic_node *, void *, struct string *, struct range *);
int logic_add_all(struct logic *, struct logic_node *, void *, struct string *, struct range *);

int logic_merge_and(struct logic *, struct logic_node *, struct logic_node *);
int logic_merge_or(struct logic *, struct logic_node *, struct logic_node *);
int logic_merge_and_or(struct logic *, struct logic_node *, struct logic_node *);

int range_and_merge(struct range *, struct range *);
int range_or_merge(struct range *, struct range *);

int logic_var_create(struct logic_var * var, void * data, struct string * name, struct range * range, struct logic * logic) {
    int status = 0;

    var->data = data;
    if(range_copy(&var->range, range)) {
        status = panic("failed to copy range object");
    } else {
        var->name = map_search(&logic->map, name->string);
        if(!var->name) {
            if(strbuf_strcpy(&logic->strbuf, name->string, name->length)) {
                status = panic("failed to strcpy strbuf object");
            } else {
                var->name = strbuf_string(&logic->strbuf);
                if(!var->name) {
                    status = panic("failed to string strbuf object");
                } else if(map_insert(&logic->map, var->name->string, var->name)) {
                    status = panic("failed to insert map object");
                }
            }
        }
    }

    return status;
}

void logic_var_destroy(struct logic_var * var) {
    range_destroy(&var->range);
}

int logic_var_merge(struct logic_var * var, enum logic_type type, struct range * range) {
    int status = 0;
    struct range result;

    switch(type) {
        case logic_and:
            if(range_and(&result, &var->range, range))
                status = panic("failed to and range object");
            break;
        case logic_or:
            if(range_or(&result, &var->range, range))
                status = panic("failed to or range object");
            break;
        default:
            status = panic("invalid type - %d", type);
            break;
    }

    if(!status) {
        range_destroy(&var->range);
        var->range = result;
    }

    return status;
}

int logic_node_var_create(struct logic * logic, void * data, struct string * name, struct range * range, struct logic_node ** result) {
    int status = 0;
    struct logic_node * node;

    node = pool_get(logic->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->type = logic_var;
        if(logic_var_create(&node->var, data, name, range, logic)) {
            status = panic("failed to create logic var object");
        } else {
            *result = node;
        }
        if(status)
            pool_put(logic->pool, node);
    }

    return status;
}

int logic_node_create(struct logic * logic, enum logic_type type, struct logic_node ** result) {
    int status = 0;
    struct logic_node * node;

    node = pool_get(logic->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->type = type;
        if(node->type < logic_and || node->type > logic_and_or) {
            status = panic("invalid type - %d", node->type);
        } else if(list_create(&node->list, logic->list.pool)) {
            status = panic("failed to create list object");
        } else {
            *result = node;
        }
        if(status)
            pool_put(logic->pool, node);
    }

    return status;
}

void logic_node_destroy(struct logic * logic, struct logic_node * node) {
    struct logic_node * iter;

    switch(node->type) {
        case logic_var:
            logic_var_destroy(&node->var);
            break;
        case logic_and:
        case logic_or:
        case logic_and_or:
            iter = list_pop(&node->list);
            while(iter) {
                logic_node_destroy(logic, iter);
                iter = list_pop(&node->list);
            }
            list_destroy(&node->list);
            break;
    }

    pool_put(logic->pool, node);
}

int logic_node_copy(struct logic * logic, struct logic_node * node, struct logic_node ** result) {
    int status = 0;
    struct logic_node * op;
    struct logic_node * var;
    struct logic_node * iter;

    switch(node->type) {
        case logic_var:
            if(logic_node_var_create(logic, node->var.data, node->var.name, &node->var.range, &var)) {
                status = panic("failed to create logic node var object");
            } else {
                *result = var;
            }
            break;
        case logic_and:
        case logic_or:
        case logic_and_or:
            if(logic_node_create(logic, node->type, &op)) {
                status = panic("failed to create logic node object");
            } else {
                iter = list_start(&node->list);
                while(iter && !status) {
                    if(logic_node_copy(logic, iter, &var)) {
                        status = panic("failed to copy logic node object");
                    } else {
                        if(list_push(&op->list, var))
                            status = panic("failed to push list object");
                        if(status)
                            logic_node_destroy(logic, var);
                    }
                    iter = list_next(&node->list);
                }
                if(status) {
                    logic_node_destroy(logic, op);
                } else {
                    *result = op;
                }
            }
            break;
        default:
            status = panic("invalid type - %d", node->type);
            break;
    }

    return status;
}

struct logic_node * logic_node_search(struct logic_node * node, struct string * name) {
    struct logic_node * iter = NULL;

    if(node->type >= logic_and && node->type <= logic_and_or) {
        iter = list_start(&node->list);
        while(iter) {
            if(iter->type == logic_var && !strcmp(iter->var.name->string, name->string))
                break;
            iter = list_next(&node->list);
        }
    }

    return iter;
}

void logic_node_print(struct logic_node * node, int indent) {
    int i;
    struct logic_node * iter;

    for(i = 0; i < indent; i++)
        fputs("    ", stdout);

    switch(node->type) {
        case logic_var:
            fprintf(stdout, "VAR %p, %s, ", node->var.data, node->var.name->string);
            range_print(&node->var.range);
            break;
        case logic_and:
            fprintf(stdout, "AND\n");
            iter = list_start(&node->list);
            while(iter) {
                logic_node_print(iter, indent + 1);
                iter = list_next(&node->list);
            }
            break;
        case logic_or:
        case logic_and_or:
            fprintf(stdout, "OR\n");
            iter = list_start(&node->list);
            while(iter) {
                logic_node_print(iter, indent + 1);
                iter = list_next(&node->list);
            }
            break;
    }
}

int logic_create(struct logic * logic, size_t size, struct heap * heap) {
    int status = 0;

    logic->pool = heap_pool(heap, sizeof(struct logic_node));
    if(!logic->pool) {
        status = panic("failed to pool heap object");
    } else {
        if(list_create(&logic->list, heap->list_pool)) {
            status = panic("failed to create list object");
        } else {
            if(map_create(&logic->map, (map_compare_cb) strcmp, heap->map_pool)) {
                status = panic("failed to create map object");
            } else {
                if(strbuf_create(&logic->strbuf, size))
                    status = panic("failed to create strbuf object");
                if(status)
                    map_destroy(&logic->map);
            }
            if(status)
                list_destroy(&logic->list);
        }
    }

    return status;
}

void logic_destroy(struct logic * logic) {
    logic_clear(logic);
    strbuf_destroy(&logic->strbuf);
    map_destroy(&logic->map);
    list_destroy(&logic->list);
}

void logic_clear(struct logic * logic) {
    struct logic_node * iter;

    iter = list_pop(&logic->list);
    while(iter) {
        logic_node_destroy(logic, iter);
        iter = list_pop(&logic->list);
    }
    map_clear(&logic->map);
    strbuf_clear(&logic->strbuf);
}

int logic_add_one(struct logic * logic, struct logic_node * op, void * data, struct string * name, struct range * range) {
    int status = 0;
    struct logic_node * var;

    var = logic_node_search(op, name);
    if(var) {
        if(logic_var_merge(&var->var, op->type, range))
            status = panic("failed to merge logic var object");
    } else {
        if(logic_node_var_create(logic, data, name, range, &var)) {
            status = panic("failed to create logic node var object");
        } else {
            if(list_push(&op->list, var))
                status = panic("failed to push list object");
            if(status)
                logic_node_destroy(logic, var);
        }
    }

    return status;
}

int logic_add_all(struct logic * logic, struct logic_node * op, void * data, struct string * name, struct range * range) {
    int status = 0;
    struct logic_node * node;

    node = list_start(&op->list);
    while(node && !status) {
        if(node->type != logic_and) {
            status = panic("invalid type - %d", node->type);
        } else if(logic_add_one(logic, node, data, name, range)) {
            status = panic("failed to add var one logic object");
        }
        node = list_next(&op->list);
    }

    return status;
}

int logic_push_var(struct logic * logic, void * data, struct string * name, struct range * range) {
    int status = 0;
    struct logic_node * op;

    op = list_pop(&logic->list);
    if(!op) {
        status = panic("missing operator");
    } else {
        switch(op->type) {
            case logic_and:
            case logic_or:
                if(logic_add_one(logic, op, data, name, range))
                    status = panic("failed to add var one logic object");
                break;
            case logic_and_or:
                if(logic_add_all(logic, op, data, name, range))
                    status = panic("failed to add var all logic object");
                break;
            default:
                status = panic("invalid type - %d", op->type);
                break;
        }
        if(list_push(&logic->list, op))
            status = panic("failed to push list object");
    }

    return status;
}

int logic_push_op(struct logic * logic, enum logic_type type) {
    int status = 0;
    struct logic_node * op;

    if(logic_node_create(logic, type, &op)) {
        status = panic("failed to create logic node object");
    } else {
        if(list_push(&logic->list, op))
            status = panic("failed to push list object");
        if(status)
            logic_node_destroy(logic, op);
    }

    return status;
}

int logic_merge_and(struct logic * logic, struct logic_node * parent, struct logic_node * child) {
    int status = 0;
    struct logic_node * iter;

    if(child->type == logic_var) {
        if(logic_add_one(logic, parent, child->var.data, child->var.name, &child->var.range))
            status = panic("failed to add var one logic object");
    } else if(child->type == logic_and) {
        iter = list_start(&child->list);
        while(iter && !status) {
            if(logic_merge_and(logic, parent, iter))
                status = panic("failed to merge and logic object");
            iter = list_next(&child->list);
        }
    } else {
        status = panic("invalid type - %d", child->type);
    }

    return status;
}

int logic_merge_or(struct logic * logic, struct logic_node * parent, struct logic_node * child) {
    int status = 0;
    struct logic_node * copy;

    if(child->type == logic_var) {
        if(logic_add_one(logic, parent, child->var.data, child->var.name, &child->var.range))
            status = panic("failed to add var one logic object");
    } else if(child->type == logic_and) {
        if(logic_node_copy(logic, child, &copy)) {
            status = panic("failed to copy logic node object");
        } else {
            if(list_push(&parent->list, copy))
                status = panic("failed to push list object");
            if(status)
                logic_node_destroy(logic, copy);
        }
    } else {
        status = panic("invalid type - %d", child->type);
    }

    return status;
}

int logic_merge_and_or(struct logic * logic, struct logic_node * parent, struct logic_node * child) {
    int status = 0;
    struct logic_node * iter;

    if(child->type == logic_var) {
        if(logic_add_all(logic, parent, child->var.data, child->var.name, &child->var.range))
            status = panic("failed to add var all logic object");
    } else if(child->type == logic_and) {
        iter = list_start(&child->list);
        while(iter && !status) {
            if(logic_merge_and_or(logic, parent, iter))
                status = panic("failed to merge and or logic object");
            iter = list_next(&child->list);
        }
    } else {
        status = panic("invalid type - %d", child->type);
    }

    return status;
}

int logic_pop_op(struct logic * logic) {
    int status = 0;
    struct logic_node * child;
    struct logic_node * parent;
    struct logic_node * node = NULL;
    struct logic_node * iter;
    struct logic_node * cross;
    struct logic_node * copy;

    child = list_pop(&logic->list);
    if(!child) {
        status = panic("failed to pop list object");
    } else {
        parent = list_pop(&logic->list);
        if(!parent) {
            status = panic("failed to pop list object");
        } else {
            switch(parent->type) {
                case logic_and:
                    switch(child->type) {
                        case logic_and:
                            if(logic_node_copy(logic, parent, &node)) {
                                status = panic("failed to copy logic node object");
                            } else {
                                if(logic_merge_and(logic, node, child)) {
                                    status = panic("failed to merge and logic object");
                                } else if(list_push(&logic->list, node)) {
                                    status = panic("failed to push list object");
                                }
                                if(status)
                                    logic_node_destroy(logic, node);
                            }
                            break;
                        case logic_or:
                            if(logic_node_create(logic, logic_and_or, &node)) {
                                status = panic("failed to create logic node object");
                            } else {
                                if(list_push(&logic->list, node)) {
                                    status = panic("failed to push list object");
                                } else {
                                    iter = list_start(&child->list);
                                    while(iter && !status) {
                                        if(logic_node_copy(logic, parent, &copy)) {
                                            status = panic("failed to copy logic node object");
                                        } else {
                                            if(logic_merge_and(logic, copy, iter)) {
                                                status = panic("failed to merge and logic object");
                                            } else if(list_push(&node->list, copy)) {
                                                status = panic("failed to push list object");
                                            }
                                            if(status)
                                                logic_node_destroy(logic, copy);
                                        }
                                        iter = list_next(&child->list);
                                    }
                                    if(status)
                                        list_pop(&logic->list);
                                }
                                if(status)
                                    logic_node_destroy(logic, node);
                            }
                            break;
                        case logic_and_or:
                            if(logic_node_create(logic, logic_and_or, &node)) {
                                status = panic("failed to create logic node object");
                            } else {
                                if(list_push(&logic->list, node)) {
                                    status = panic("failed to push list object");
                                } else {
                                    iter = list_start(&child->list);
                                    while(iter && !status) {
                                        if(iter->type != logic_and) {
                                            status = panic("invalid type - %d", iter->type);
                                        } else if(logic_node_copy(logic, iter, &copy)) {
                                            status = panic("failed to copy logic node object");
                                        } else {
                                            if(logic_merge_and(logic, copy, parent)) {
                                                status = panic("failed to merge and logic object");
                                            } else if(list_push(&node->list, copy)) {
                                                status = panic("failed to push list object");
                                            }
                                            if(status)
                                                logic_node_destroy(logic, copy);
                                        }
                                        iter = list_next(&child->list);
                                    }
                                    if(status)
                                        list_pop(&logic->list);
                                }
                                if(status)
                                    logic_node_destroy(logic, node);
                            }
                            break;
                        default:
                            status = panic("invalid type - %d", child->type);
                            break;
                    }
                    break;
                case logic_or:
                    switch(child->type) {
                        case logic_and:
                            if(logic_node_copy(logic, parent, &node)) {
                                status = panic("failed to copy logic node object");
                            } else {
                                if(logic_merge_or(logic, node, child)) {
                                    status = panic("failed to merge or logic object");
                                } else if(list_push(&logic->list, node)) {
                                    status = panic("failed to push list object");
                                }
                                if(status)
                                    logic_node_destroy(logic, node);
                            }
                            break;
                        case logic_or:
                        case logic_and_or:
                            if(logic_node_copy(logic, parent, &node)) {
                                status = panic("failed to copy logic node object");
                            } else {
                                if(list_push(&logic->list, node)) {
                                    status = panic("failed to push list object");
                                } else {
                                    iter = list_start(&child->list);
                                    while(iter && !status) {
                                        if(logic_merge_or(logic, node, iter))
                                            status = panic("failed to merge or logic object");
                                        iter = list_next(&child->list);
                                    }
                                    if(status)
                                        list_pop(&logic->list);
                                }
                                if(status)
                                    logic_node_destroy(logic, node);
                            }
                            break;
                        default:
                            status = panic("invalid type - %d", child->type);
                            break;
                    }
                    break;
                case logic_and_or:
                    switch(child->type) {
                        case logic_and:
                            if(logic_node_copy(logic, parent, &node)) {
                                status = panic("failed to copy logic node object");
                            } else {
                                if(logic_merge_and_or(logic, node, child)) {
                                    status = panic("failed to merge and or logic object");
                                } else if(list_push(&logic->list, node)) {
                                    status = panic("failed to push list object");
                                }
                                if(status)
                                    logic_node_destroy(logic, node);
                            }
                            break;
                        case logic_or:
                        case logic_and_or:
                            if(logic_node_create(logic, logic_and_or, &node)) {
                                status = panic("failed to create logic node object");
                            } else {
                                if(list_push(&logic->list, node)) {
                                    status = panic("failed to push list object");
                                } else {
                                    iter = list_start(&parent->list);
                                    while(iter && !status) {
                                        cross = list_start(&child->list);
                                        while(cross && !status) {
                                            if(iter->type != logic_and) {
                                                status = panic("invalid type - %d", iter->type);
                                            } else if(logic_node_copy(logic, iter, &copy)) {
                                                status = panic("failed to copy logic node object");
                                            } else {
                                                if(logic_merge_and(logic, copy, cross)) {
                                                    status = panic("failed to merge and logic object");
                                                } else if(list_push(&node->list, copy)) {
                                                    status = panic("failed to push list object");
                                                }
                                                if(status)
                                                    logic_node_destroy(logic, copy);
                                            }
                                            cross = list_next(&child->list);
                                        }
                                        iter = list_next(&parent->list);
                                    }
                                    if(status)
                                        list_pop(&logic->list);
                                }
                                if(status)
                                    logic_node_destroy(logic, node);
                            }
                            break;
                        default:
                            status = panic("invalid type - %d", child->type);
                            break;
                    }
                    break;
                default:
                    status = panic("invalid type - %d", parent->type);
                    break;
            }
            logic_node_destroy(logic, parent);
        }
        logic_node_destroy(logic, child);
    }

    return status;
}

int range_and_merge(struct range * result, struct range * range) {
    int status = 0;
    struct range merge;

    if(range_and(&merge, result, range)) {
        status = panic("failed to and range object");
    } else {
        range_destroy(result);
        *result = merge;
    }

    return status;
}

int range_or_merge(struct range * result, struct range * range) {
    int status = 0;
    struct range merge;

    if(range_or(&merge, result, range)) {
        status = panic("failed to or range object");
    } else {
        range_destroy(result);
        *result = merge;
    }

    return status;
}

int logic_search(struct logic * logic, struct string * name, struct range * range) {
    int status = 0;
    struct logic_node * root;
    struct logic_node * node;
    struct logic_node * iter;
    struct range result;

    root = list_start(&logic->list);
    if(root) {
        switch(root->type) {
            case logic_and:
                node = logic_node_search(root, name);
                if(node && range_and_merge(range, &node->var.range))
                    status = panic("failed to and merge range object");
                break;
            case logic_or:
            case logic_and_or:
                if(range_create(&result, range->pool)) {
                    status = panic("failed to create range object");
                } else {
                    iter = list_start(&root->list);
                    while(iter && !status) {
                        switch(iter->type) {
                            case logic_var:
                                if(!strcmp(iter->var.name->string, name->string) && range_or_merge(&result, &iter->var.range))
                                    status = panic("failed to or merge range object");
                                break;
                            case logic_and:
                                node = logic_node_search(iter, name);
                                if(node && range_or_merge(&result, &node->var.range))
                                    status = panic("failed to or merge range object");
                                break;
                            default:
                                status = panic("invalid type - %d", iter->type);
                                break;
                        }
                        iter = list_next(&root->list);
                    }
                    if(status) {
                        /* skip on error */
                    } else if(range_and_merge(range, &result)) {
                        status = panic("failed to and merge range object");
                    }
                    range_destroy(&result);
                }
                break;
            default:
                status = panic("invalid type - %d", root->type);
                break;
        }
    }

    return status;
}

void logic_print(struct logic * logic) {
    struct logic_node * node;

    node = list_start(&logic->list);
    if(!node) {
        fprintf(stdout, "logic is empty\n");
    } else {
        logic_node_print(node, 0);
    }
}
