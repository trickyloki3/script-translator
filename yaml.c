#include "yaml.h"

#include "yaml_parser.h"
#include "yaml_scanner.h"

int yaml_node_create(struct yaml *, int, size_t, struct string *, struct yaml_node **);
void yaml_node_destroy(struct yaml *, struct yaml_node *);

int yaml_parse_loop(struct yaml *, yyscan_t, yamlpstate *);

int yaml_scalar(struct yaml *, struct yaml_node *);

static inline int yaml_start(struct yaml *, struct yaml_node *);
static inline int yaml_next(struct yaml *, struct yaml_node *);
static inline int yaml_end(struct yaml *, struct yaml_node *);

int yaml_node_create(struct yaml * yaml, int type, size_t scope, struct string * string, struct yaml_node ** result) {
    int status = 0;
    struct yaml_node * node;

    node = pool_get(yaml->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->type = type;
        node->scope = scope;
        node->key = NULL;
        node->value = string;
        node->child = NULL;
        *result = node;
    }

    return status;
}

void yaml_node_destroy(struct yaml * yaml, struct yaml_node * node) {
    pool_put(yaml->pool, node);
}

int yaml_create(struct yaml * yaml, size_t size, struct heap * heap) {
    int status = 0;

    yaml->pool = heap_pool(heap, sizeof(struct yaml_node));
    if(!yaml->pool) {
        status = panic("failed to pool heap object");
    } else {
        if(strbuf_create(&yaml->strbuf, size)) {
            status = panic("failed to create strbuf object");
        } else {
            if(strbuf_create(&yaml->scalar, size)) {
                status = panic("failed to create strbuf object");
            } else {
                if(list_create(&yaml->list, heap->list_pool)) {
                    status = panic("failed to create list object");
                } else {
                    yaml->line = 0;
                    yaml->space = 0;
                    yaml->root = NULL;
                    yaml->stack = NULL;
                    yaml->indent = NULL;
                }
                if(status)
                    strbuf_destroy(&yaml->scalar);
            }
            if(status)
                strbuf_destroy(&yaml->strbuf);
        }
    }

    return status;
}

void yaml_destroy(struct yaml * yaml) {
    list_destroy(&yaml->list);
    strbuf_destroy(&yaml->scalar);
    strbuf_destroy(&yaml->strbuf);
}

int yaml_parse(struct yaml * yaml, const char * path, size_t size, event_cb callback, void * context) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    yamlpstate * parser;
    YY_BUFFER_STATE buffer;

    yaml->callback = callback;
    yaml->context = context;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(yamllex_init_extra(yaml, &scanner)) {
            status = panic("failed to create scanner object");
        } else {
            parser = yamlpstate_new();
            if(!parser) {
                status = panic("failed to create parser object");
            } else {
                buffer = yaml_create_buffer(file, size, scanner);
                if(!buffer) {
                    status = panic("faield to create buffer state object");
                } else {
                    yamlpush_buffer_state(buffer, scanner);
                    if(yaml_parse_loop(yaml, scanner, parser))
                        status = panic("failed to parse loop yaml object");
                    yamlpop_buffer_state(scanner);
                }
                yamlpstate_delete(parser);
            }
            yamllex_destroy(scanner);
        }
        fclose(file);
    }

    return status;
}

int yaml_parse_loop(struct yaml * yaml, yyscan_t scanner, yamlpstate * parser) {
    int status = 0;

    YAMLSTYPE value;
    YAMLLTYPE location;
    int token;
    int state;
    struct yaml_node * node;

    do {
        token = yamllex(&value, &location, scanner);
        if(token < 0) {
            status = panic("failed to get the next token");
        } else {
            state = yamlpush_parse(parser, token, &value, &location, yaml);
            if(state && state != YYPUSH_MORE)
                status = panic("failed to parse the current token");
        }
    } while(token && state == YYPUSH_MORE && !status);

    /*
     * reset yaml object to initial state
     */
    yaml->indent = NULL;

    while(yaml->stack) {
        node = yaml->stack;
        yaml->stack = yaml->stack->child;
        yaml_node_destroy(yaml, node);
    }

    while(yaml->root) {
        node = yaml->root;
        yaml->root = yaml->root->child;
        yaml_node_destroy(yaml, node);
    }

    yaml->space = 0;
    yaml->line = 0;

    node = list_pop(&yaml->list);
    while(node) {
        yaml_node_destroy(yaml, node);
        node = list_pop(&yaml->list);
    }

    strbuf_clear(&yaml->scalar);
    strbuf_clear(&yaml->strbuf);

    return status;
}

int yaml_token(struct yaml * yaml, int type, size_t scope, struct string * string, struct yaml_node ** result) {
    int status = 0;
    struct yaml_node * node;

    if(yaml_node_create(yaml, type, scope, string, &node)) {
        status = panic("failed to create yaml node object");
    } else {
        if(list_push(&yaml->list, node)) {
            status = panic("failed to push list object");
        } else {
            /*
             * update the indentation for the
             * next sequence,  map, or scalar
             */
            if(node->type == yaml_s_indent) {
                yaml->indent = node;
            } else if(node->type == yaml_b_break) {
                yaml->indent = NULL;
            } else if(node->type == yaml_s_separate_in_line) {
                if(yaml->indent)
                    node->scope += yaml->indent->scope + 1;
                yaml->indent = node;
            }
        }
        if(status) {
            yaml_node_destroy(yaml, node);
        } else {
            *result = node;
        }
    }

    return status;
}

int yaml_string(struct yaml * yaml, int type, size_t scope, char * buffer, size_t length, struct yaml_node ** result) {
    int status = 0;
    struct string * string;

    if(strbuf_strcpy(&yaml->strbuf, buffer, length)) {
        status = panic("failed to strcpy strbuf object");
    } else {
        string = strbuf_string(&yaml->strbuf);
        if(!string) {
            status = panic("failed to string strbuf object");
        } else {
            status = yaml_token(yaml, type, scope, string, result);
        }
    }

    return status;
}

int yaml_literal(struct yaml * yaml, size_t scope) {
    int status = 0;

    if(yaml->stack) {
        /*
         * first non-empty line for block scalar
         */
        status = ( yaml->stack->type == yaml_c_literal ||
                   yaml->stack->type == yaml_c_folded ) &&
                   yaml->stack->scope < scope;
        if(status)
            yaml->stack->scope = scope;
    } else if(yaml->root) {
        /*
         * other non-empty line for block scalar
         */
        status = ( yaml->root->type == yaml_c_literal  ||
                   yaml->root->type == yaml_c_folded ) &&
                   yaml->root->scope <= scope;
    }

    return status;
}

int yaml_stack(struct yaml * yaml, int type) {
    int status = 0;
    size_t scope;
    struct yaml_node * node;

    if(type == yaml_c_sequence_entry || type == yaml_c_mapping_value) {
        scope = yaml->indent ? yaml->indent->scope : 0;
    } else {
        scope = yaml->stack ? yaml->stack->scope : 0;
    }

    if(yaml_node_create(yaml, type, scope, NULL, &node)) {
        status = panic("failed to create yaml node object");
    } else {
        node->child = yaml->stack;
        yaml->stack = node;
    }

    return status;
}

int yaml_scalar(struct yaml * yaml, struct yaml_node * node) {
    int status = 0;
    size_t space;

    if(node->type != yaml_nb_char) {
        status = panic("invalid scalar");
    } else {
        space = node->scope - yaml->root->scope;

        switch(yaml->root->type) {
            case yaml_c_literal:
                if(strbuf_putcn(&yaml->scalar, '\n', yaml->line))
                    status = panic("failed to putcn strbuf object");
                break;
            case yaml_c_folded:
                if(yaml->line == 1) {
                    if(strbuf_putc(&yaml->scalar, yaml->space ? '\n' : ' '))
                        status = panic("failed to putc strbuf object");
                } else if(yaml->line > 1) {
                    if(strbuf_putcn(&yaml->scalar, '\n', yaml->line - 1))
                        status = panic("failed to putcn strbuf object");
                }
                break;
        }
        if(status) {
            /* skip scalar on error */
        } else if(strbuf_putcn(&yaml->scalar, ' ', space)) {
            status = panic("failed to putcn strbuf object");
        } else if(strbuf_strcpy(&yaml->scalar, node->value->string, node->value->length)) {
            status = panic("failed to strcpy strbuf object");
        } else {
            yaml->line = node->child->scope;
            yaml->space = space;
        }
    }

    return status;
}

static inline int yaml_start(struct yaml * yaml, struct yaml_node * node) {
    int status = 0;

    if(node->type == yaml_c_sequence_entry) {
        if(yaml->callback(list_begin, NULL, yaml->context)) {
            status = panic("failed to process list start event");
        } else if(yaml_next(yaml, node)) {
            status = panic("failed to next yaml object");
        }
    } else if(node->type == yaml_c_mapping_value) {
        if(yaml->callback(map_begin, NULL, yaml->context)) {
            status = panic("failed to process map start event");
        } else if(yaml_next(yaml, node)) {
            status = panic("failed to next yaml object");
        }
    }

    return status;
}

static inline int yaml_next(struct yaml * yaml, struct yaml_node * node) {
    int status = 0;

    if(node->key && yaml->callback(scalar, node->key, yaml->context)) {
        status = panic("failed to process string event");
    } else if(node->value && yaml->callback(scalar, node->value, yaml->context)) {
        status = panic("failed to process string event");
    }

    return status;
}

static inline int yaml_end(struct yaml * yaml, struct yaml_node * node) {
    int status = 0;
    struct string * string;

    if(node->type == yaml_c_sequence_entry) {
        if(yaml->callback(list_end, NULL, yaml->context))
            status = panic("failed to process list end event");
    } else if(node->type == yaml_c_mapping_value) {
        if(yaml->callback(map_end, NULL, yaml->context))
            status = panic("failed to process map end event");
    } else if(node->type == yaml_c_literal || node->type == yaml_c_folded) {
        string = strbuf_string(&yaml->scalar);
        if(!string) {
            status = panic("failed to string strbuf object");
        } else if(yaml->callback(scalar, string, yaml->context)) {
            status = panic("failed to process string object");
        }
        yaml->space = 0;
        yaml->line = 0;
        strbuf_clear(&yaml->scalar);
    }

    return status;
}

int yaml_block(struct yaml * yaml, struct yaml_node * block) {
    int status = 0;
    struct yaml_node * list;
    struct yaml_node * node;
    struct yaml_node * peek;

    list = NULL;
    while(yaml->stack) {
        node = yaml->stack;
        yaml->stack = yaml->stack->child;
        node->child = list;
        list = node;
    }
    yaml->stack = list;

    if(!yaml->root) {
        while(yaml->stack && !status) {
            if(yaml_start(yaml, yaml->stack)) {
                status = panic("failed to start yaml object");
            } else {
                node = yaml->stack;
                yaml->stack = yaml->stack->child;
                node->child = yaml->root;
                yaml->root = node;
            }
        }
    } else {
        while(yaml->root && yaml->root->scope > block->scope && !status) {
            if(yaml_end(yaml, yaml->root)) {
                status = panic("failed to end yaml object");
            } else {
                node = yaml->root;
                yaml->root = yaml->root->child;
                yaml_node_destroy(yaml, node);
            }
        }
        if(status) {
            /* skip on error */
        } else if(!yaml->root) {
            status = panic("invalid scope - %zu", block->scope);
        } else if(yaml->root->type == yaml_c_literal || yaml->root->type == yaml_c_folded) {
            if(yaml_scalar(yaml, block))
                status = panic("failed to scalar yaml object");
        } else if(yaml->root->type == yaml_c_sequence_entry || yaml->root->type == yaml_c_mapping_value) {
            if(yaml->stack->type != yaml->root->type) {
                status = panic("invalid sequence entry or map value");
            } else if(yaml->stack->scope != yaml->root->scope) {
                status = panic("invalid scope - %d", yaml->stack->scope);
            } else {
                if(yaml_next(yaml, yaml->stack)) {
                    status = panic("failed to next yaml object");
                } else {
                    node = yaml->stack;
                    yaml->stack = yaml->stack->child;
                    yaml_node_destroy(yaml, node);

                    while(yaml->stack && !status) {
                        if(yaml_start(yaml, yaml->stack)) {
                            status = panic("failed to start yaml object");
                        } else {
                            node = yaml->stack;
                            yaml->stack = yaml->stack->child;
                            node->child = yaml->root;
                            yaml->root = node;
                        }
                    }
                }
            }
        }
    }

    /*
     * bison is a lalr(1) parser
     * save the look-ahead token
     */
    peek = list_pop(&yaml->list);
    if(peek) {
        node = list_pop(&yaml->list);
        while(node) {
            yaml_node_destroy(yaml, node);
            node = list_pop(&yaml->list);
        }

        if(list_push(&yaml->list, peek)) {
            status = panic("failed to push list object");
        } else {
            if(peek->value) {
                strbuf_clear_move(&yaml->strbuf, peek->value->string, peek->value->length);
                peek->value = strbuf_string(&yaml->strbuf);
                if(!peek->value)
                    status = panic("failed to string strbuf object");
            } else {
                strbuf_clear(&yaml->strbuf);
            }
            if(status)
                list_pop(&yaml->list);
        }
        if(status)
            yaml_node_destroy(yaml, peek);
    }

    return status;
}

int yaml_document(struct yaml * yaml) {
    int status = 0;
    struct yaml_node * node;

    while(yaml->root && !status) {
        if(yaml_end(yaml, yaml->root)) {
            status = panic("failed to end yaml object");
        } else {
            node = yaml->root;
            yaml->root = yaml->root->child;
            yaml_node_destroy(yaml, node);
        }
    }

    return status;
}
