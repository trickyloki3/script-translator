#include "logic.h"

int logic_var_create(struct logic_var *, void *, sstring, struct range *, struct sector_list *);
void logic_var_destroy(struct logic_var *);
int logic_var_merge(struct logic_var *, enum logic_type, struct range *);

int logic_node_var_create(struct logic *, void *, sstring, struct range *, struct logic_node **);
int logic_node_create(struct logic *, enum logic_type, struct logic_node **);
void logic_node_destroy(struct logic *, struct logic_node *);
int logic_node_copy(struct logic *, struct logic_node *, struct logic_node **);
struct logic_node * logic_node_search(struct logic_node *, sstring);
void logic_node_print(struct logic_node *, int);

int logic_add_var_one(struct logic *, struct logic_node *, void *, sstring, struct range *);
int logic_add_var_all(struct logic *, struct logic_node *, void *, sstring, struct range *);

int logic_merge_var(struct logic *, struct logic_node *, struct logic_node *);
int logic_merge_and_re(struct logic *, struct logic_node *, struct logic_node *);
int logic_merge_and(struct logic *, struct logic_node *, struct logic_node *);
int logic_merge_or_re(struct logic *, struct logic_node *, struct logic_node *);
int logic_merge_or(struct logic *, struct logic_node *, struct logic_node *);

int logic_var_create(struct logic_var * var, void * data, sstring name, struct range * range, struct sector_list * sector_list) {
    int status = 0;

    var->data = data;
    if(sstring_create(&var->name, name, sstring_size(name), sector_list)) {
        status = panic("failed to create string object");
    } else {
        if(range_copy(&var->range, range))
            status = panic("failed to copy range object");
        if(status)
            sstring_destroy(var->name);
    }

    return status;
}

void logic_var_destroy(struct logic_var * var) {
    range_destroy(&var->range);
    sstring_destroy(var->name);
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

int logic_node_var_create(struct logic * logic, void * data, sstring name, struct range * range, struct logic_node ** result) {
    int status = 0;
    struct logic_node * node;

    node = pool_get(logic->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->type = logic_var;
        if(logic_var_create(&node->var, data, name, range, logic->sector_list)) {
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

struct logic_node * logic_node_search(struct logic_node * node, sstring name) {
    struct logic_node * iter = NULL;

    if(node->type >= logic_and && node->type <= logic_and_or) {
        iter = list_start(&node->list);
        while(iter) {
            if(iter->type == logic_var && !strcmp(iter->var.name, name))
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
            fprintf(stdout, "VAR %p, %s, ", node->var.data, node->var.name);
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

int logic_create(struct logic * logic, struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;

    logic->pool = pool_map_get(pool_map, sizeof(struct logic_node));
    if(!logic->pool) {
        status = panic("failed to get pool map object");
    } else {
        if(list_create(&logic->list, pool_map_get(pool_map, sizeof(struct list_node)))) {
            status = panic("failed to create list object");
        } else {
            logic->sector_list = sector_list;
        }
    }

    return status;
}

void logic_destroy(struct logic * logic) {
    struct logic_node * iter;

    iter = list_pop(&logic->list);
    while(iter) {
        logic_node_destroy(logic, iter);
        iter = list_pop(&logic->list);
    }

    list_destroy(&logic->list);
}

int logic_add_var_one(struct logic * logic, struct logic_node * op, void * data, sstring name, struct range * range) {
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

int logic_add_var_all(struct logic * logic, struct logic_node * op, void * data, sstring name, struct range * range) {
    int status = 0;
    struct logic_node * node;

    node = list_start(&op->list);
    while(node && !status) {
        if(node->type != logic_and) {
            status = panic("invalid type - %d", node->type);
        } else if(logic_add_var_one(logic, node, data, name, range)) {
            status = panic("failed to add var logic object");
        }
        node = list_next(&op->list);
    }

    return status;
}

int logic_push_var(struct logic * logic, void * data, sstring name, struct range * range) {
    int status = 0;
    struct logic_node * op;

    op = list_start(&logic->list);
    if(!op) {
        status = panic("missing operator");
    } else {
        switch(op->type) {
            case logic_and:
            case logic_or:
                if(logic_add_var_one(logic, op, data, name, range))
                    status = panic("failed to add var logic object");
                break;
            case logic_and_or:
                if(logic_add_var_all(logic, op, data, name, range))
                    status = panic("failed to add var all logic object");
                break;
            default:
                status = panic("invalid type - %d", op->type);
                break;
        }
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

int logic_merge_var(struct logic * logic, struct logic_node * op, struct logic_node * var) {
    int status = 0;
    struct logic_node * node;

    node = logic_node_search(op, var->var.name);
    if(node) {
        if(logic_var_merge(&node->var, op->type, &var->var.range))
            status = panic("failed to merge logic var object");
        logic_node_destroy(logic, var);
    } else {
        if(list_push(&op->list, var))
            status = panic("failed to push list object");
        if(status)
            logic_node_destroy(logic, var);
    }

    return status;
}

int logic_merge_and_re(struct logic * logic, struct logic_node * parent, struct logic_node * child) {
    int status = 0;
    struct logic_node * node;

    switch(child->type) {
        case logic_var:
            return logic_merge_var(logic, parent, child);
        case logic_and:
            node = list_pop(&child->list);
            while(node && !status) {
                if(logic_merge_and_re(logic, parent, node)) {
                    status = panic("failed to merge and logic object");
                } else {
                    node = list_pop(&child->list);
                }
            }
            break;
        default:
            status = panic("invalid type - %d", child->type);
            break;
    }
    logic_node_destroy(logic, child);

    return status;
}

int logic_merge_and(struct logic * logic, struct logic_node * parent, struct logic_node * child) {
    int status = 0;
    struct logic_node * node;
    struct logic_node * copy;
    struct logic_node * var;
    struct logic_node * op;

    switch(child->type) {
        case logic_and:
            if(logic_merge_and_re(logic, parent, child))
                status = panic("failed to merge and logic object");
            if(status || list_push(&logic->list, parent))
                logic_node_destroy(logic, parent);
            break;
        case logic_or:
            if(logic_node_create(logic, logic_and_or, &op)) {
                status = panic("failed to create logic node object");
            } else {
                node = list_pop(&child->list);
                while(node && !status) {
                    if(logic_node_copy(logic, parent, &copy)) {
                        status = panic("failed to copy logic node object");
                    } else {
                        if(logic_merge_and_re(logic, copy, node)) {
                            status = panic("failed to merge and logic object");
                        } else if(list_push(&op->list, copy)) {
                            status = panic("failed to push list object");
                        } else {
                            node = list_pop(&child->list);
                        }
                        if(status)
                            logic_node_destroy(logic, copy);
                        continue;
                    }
                    if(status)
                        logic_node_destroy(logic, node);
                }
                if(status || list_push(&logic->list, op))
                    logic_node_destroy(logic, op);
            }
            logic_node_destroy(logic, parent);
            logic_node_destroy(logic, child);
            break;
        case logic_and_or:
            var = list_start(&parent->list);
            while(var && !status) {
                op = list_start(&child->list);
                while(op && !status) {
                    if(op->type != logic_and) {
                        status = panic("invalid type - %d", op->type);
                    } else if(logic_node_copy(logic, var, &copy)) {
                        status = panic("failed to copy logic node object");
                    } else if(logic_merge_and_re(logic, op, copy)) {
                        status = panic("failed to merge and logic object");
                    }
                    op = list_next(&child->list);
                }
                var = list_next(&parent->list);
            }
            logic_node_destroy(logic, parent);
            if(status || list_push(&logic->list, child))
                logic_node_destroy(logic, child);
            break;
        default:
            logic_node_destroy(logic, parent);
            logic_node_destroy(logic, child);
            status = panic("invalid type - %d", child->type);
            break;
    }

    return status;
}

int logic_merge_or_re(struct logic * logic, struct logic_node * parent, struct logic_node * child) {
    int status = 0;

    switch(child->type) {
        case logic_var:
            return logic_merge_var(logic, parent, child);
        case logic_and:
            if(list_push(&parent->list, child))
                status = panic("failed to push list object");
            break;
        default:
            status = panic("invalid type - %d", child->type);
            break;
    }
    if(status)
        logic_node_destroy(logic, child);

    return status;
}

int logic_merge_or(struct logic * logic, struct logic_node * parent, struct logic_node * child) {
    int status = 0;
    struct logic_node * node;

    switch(child->type) {
        case logic_and:
            if(logic_merge_or_re(logic, parent, child))
                status = panic("failed to merge or logic object");
            if(status || list_push(&logic->list, parent))
                logic_node_destroy(logic, parent);
            break;
        case logic_or:
        case logic_and_or:
            node = list_pop(&child->list);
            while(node && !status) {
                if(logic_merge_or_re(logic, parent, node)) {
                    status = panic("failed to merge or logic object");
                } else {
                    node = list_pop(&child->list);
                }
            }
            logic_node_destroy(logic, child);
            if(status || list_push(&logic->list, parent))
                logic_node_destroy(logic, parent);
            break;
        default:
            logic_node_destroy(logic, parent);
            logic_node_destroy(logic, child);
            status = panic("invalid type - %d", child->type);
            break;
    }

    return status;
}

int logic_pop_op(struct logic * logic) {
    int status = 0;
    struct logic_node * child;
    struct logic_node * parent;

    child = list_pop(&logic->list);
    if(!child) {
        status = panic("missing operator");
    } else {
        parent = list_pop(&logic->list);
        if(!parent) {
            if(list_push(&logic->list, child))
                status = panic("failed to push list object");
            if(status)
                logic_node_destroy(logic, child);
        } else {
            switch(parent->type) {
                case logic_and:
                    if(logic_merge_and(logic, parent, child))
                        status = panic("failed to merge and logic object");
                    break;
                case logic_or:
                    if(logic_merge_or(logic, parent, child))
                        status = panic("failed to merge or logic object");
                    break;
                case logic_and_or:
                    break;
                default:
                    logic_node_destroy(logic, parent);
                    logic_node_destroy(logic, child);
                    status = panic("invalid type - %d", parent->type);
                    break;
            }
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
