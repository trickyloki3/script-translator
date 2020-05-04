#include "yaml.h"

#include "yaml_scanner.h"

int yaml_push(struct yaml *, enum yaml_type, int);
int yaml_pop(struct yaml *, int);

int yaml_document(struct yaml *);
int yaml_block(struct yaml *, int);
int yaml_sequence(struct yaml *, int);
int yaml_map(struct yaml *, int);
int yaml_scalar(struct yaml *, int, int);

int yaml_create(struct yaml * yaml, size_t size, struct heap * heap) {
    int status = 0;

    yaml->pool = heap_pool(heap, sizeof(struct yaml_node));
    if(!yaml->pool) {
        status = panic("failed to pool heap object");
    } else if(strbuf_create(&yaml->strbuf, size)) {
        status = panic("failed to create strbuf object");
    }

    return status;
}

void yaml_destroy(struct yaml * yaml) {
    strbuf_destroy(&yaml->strbuf);
}

int yaml_parse(struct yaml * yaml, const char * path, event_cb callback, void * context) {
    int status = 0;

    FILE * file;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(yamllex_init_extra(yaml, &yaml->scanner)) {
            status = panic("failed to create scanner object");
        } else {
            yamlrestart(file, yaml->scanner);

            yaml->scalar = 0;
            yaml->stack = NULL;
            yaml->callback = callback;
            yaml->context = context;

            if(yaml_document(yaml)) {
                status = panic("failed to document yaml object");
            } else if(yaml_pop(yaml, -1)) {
                status = panic("failed to pop yaml object");
            }

            yamllex_destroy(yaml->scanner);
        }
        fclose(file);
    }

    strbuf_clear(&yaml->strbuf);

    return status;
}

int yaml_push(struct yaml * yaml, enum yaml_type type, int scope) {
    int status = 0;
    struct yaml_node * node;

    if(yaml->stack && yaml->stack->scope >= scope) {
        status = panic("invalid scope - %d", scope);
    } else {
        node = pool_get(yaml->pool);
        if(!node) {
            status = panic("out of memory");
        } else {
            node->type = type;
            node->scope = scope;
            node->next = yaml->stack;
            yaml->stack = node;

            switch(type) {
                case sequence_type:
                    if(yaml->callback(event_list_start, NULL, yaml->context))
                        status = panic("failed to process list start event");
                    break;
                case map_type:
                    if(yaml->callback(event_map_start, NULL, yaml->context))
                        status = panic("failed to process map start event");
                    break;
            }
        }
    }

    return status;
}

int yaml_pop(struct yaml * yaml, int scope) {
    int status = 0;
    struct yaml_node * node;

    while(yaml->stack && yaml->stack->scope > scope && !status) {
        switch(yaml->stack->type) {
            case sequence_type:
                if(yaml->callback(event_list_end, NULL, yaml->context))
                    status = panic("failed to process list end event");
                break;
            case map_type:
                if(yaml->callback(event_map_end, NULL, yaml->context))
                    status = panic("failed to process map end event");
                break;
        }

        node = yaml->stack;
        yaml->stack = yaml->stack->next;
        pool_put(yaml->pool, node);
    }

    return status;
}

int yaml_document(struct yaml * yaml) {
    int status = 0;

    int scope;
    struct string string;

    yaml->token = yamllex(yaml->scanner);
    while(yaml->token == l_empty)
        yaml->token = yamllex(yaml->scanner);

    while(yaml->token && !status) {
        scope = 0;

        if(yaml->token == s_indent) {
            scope = yaml->space;
            yaml->token = yamllex(yaml->scanner);
        }

        if(yaml_pop(yaml, scope)) {
            status = panic("failed to pop yaml object");
        } else {
            if(yaml->stack) {
                if(yaml->stack->scope != scope) {
                    status = panic("invalid scope - %d", scope);
                } else {
                    if(yaml->stack->type == sequence_type) {
                        if(yaml->token == c_sequence_entry) {
                            yaml->token = yamllex(yaml->scanner);
                            if(yaml_sequence(yaml, scope))
                                status = panic("failed to sequence entry yaml object");
                        } else {
                            status = panic("expected sequence entry");
                        }
                    } else if(yaml->stack->type == map_type) {
                        if(yaml->token == ns_plain_one_line) {
                            string.string = yaml->string;
                            string.length = yaml->length;

                            if(yaml->callback(event_scalar, &string, yaml->context)) {
                                status = panic("failed to process scalar event");
                            } else {
                                yaml->token = yamllex(yaml->scanner);
                                if(yaml->token == c_mapping_value) {
                                    yaml->token = yamllex(yaml->scanner);
                                    if(yaml_map(yaml, scope))
                                        status = panic("failed to map entry yaml object");
                                } else {
                                    status = panic("expected mapping value");
                                }
                            }
                        } else {
                            status = panic("expected mapping key");
                        }
                    } else {
                        status = panic("invalid type - %d", yaml->stack->type);
                    }
                }
            } else if(yaml_block(yaml, scope)) {
                status = panic("failed to block yaml object");
            }
        }
    }

    return status;
}

int yaml_block(struct yaml * yaml, int scope) {
    int status = 0;

    struct string * string;

    switch(yaml->token) {
        case ns_plain_one_line:
            if(strbuf_strcpy(&yaml->strbuf, yaml->string, yaml->length)) {
                status = panic("failed to strcpy strbuf object");
            } else {
                string = strbuf_string(&yaml->strbuf);
                if(!string) {
                    status = panic("failed to string strbuf object");
                } else {
                    yaml->token = yamllex(yaml->scanner);

                    if(yaml->token == b_break) {
                        if(yaml->callback(event_scalar, string, yaml->context)) {
                            status = panic("failed to process scalar event");
                        } else {
                            strbuf_clear(&yaml->strbuf);

                            yaml->token = yamllex(yaml->scanner);
                            while(yaml->token == l_empty)
                                yaml->token = yamllex(yaml->scanner);
                        }
                    } else if(yaml->token == c_mapping_value) {
                        if(yaml_push(yaml, map_type, scope)) {
                            status = panic("failed to push yaml object");
                        } else {
                            yaml->token = yamllex(yaml->scanner);

                            if(yaml->callback(event_scalar, string, yaml->context)) {
                                status = panic("failed to process scalar event");
                            } else {
                                strbuf_clear(&yaml->strbuf);

                                if(yaml_map(yaml, scope))
                                    status = panic("failed to map entry yaml object");
                            }
                        }
                    } else {
                        status = panic("expected newline or mapping value");
                    }
                }
            }
            break;
        case c_sequence_entry:
            if(yaml_push(yaml, sequence_type, scope)) {
                status = panic("failed ot push yaml object");
            } else {
                yaml->token = yamllex(yaml->scanner);
                if(yaml_sequence(yaml, scope))
                    status = panic("failed to sequence entry yaml object");
            }
            break;
        case c_literal:
            yaml->token = yamllex(yaml->scanner);
            if(yaml_scalar(yaml, scope, 1))
                status = panic("failed to scalar yaml object");
            break;
        case c_folded:
            yaml->token = yamllex(yaml->scanner);
            if(yaml_scalar(yaml, scope, 0))
                status = panic("failed to scalar yaml object");
            break;
        default:
            status = panic("invalid token - %d", yaml->token);
            break;
    }

    return status;
}

int yaml_sequence(struct yaml * yaml, int scope) {
    int status = 0;

    if(yaml->token == s_separate_in_line) {
        scope += yaml->space + 1;
        yaml->token = yamllex(yaml->scanner);

        if(yaml_block(yaml, scope))
            status = panic("failed to block yaml object");
    } else if(yaml->token == b_break) {
        yaml->token = yamllex(yaml->scanner);
        while(yaml->token == l_empty)
            yaml->token = yamllex(yaml->scanner);

        if(yaml->token == s_indent) {
            scope = yaml->space;
            yaml->token = yamllex(yaml->scanner);

            if(yaml_block(yaml, scope))
                status = panic("failed to block yaml object");
        } else {
            status = panic("expected space");
        }
    } else {
        status = panic("expected space or newline");
    }

    return status;
}

int yaml_map(struct yaml * yaml, int scope) {
    int status = 0;

    if(yaml->token == s_separate_in_line) {
        scope += yaml->space + 1;
        yaml->token = yamllex(yaml->scanner);

        if(yaml->token == c_sequence_entry || yaml->token == c_mapping_value) {
            status = panic("map does not support compact notation");
        } else if(yaml_block(yaml, scope)) {
            status = panic("failed to block yaml object");
        }
    } else if(yaml->token == b_break) {
        yaml->token = yamllex(yaml->scanner);
        while(yaml->token == l_empty)
            yaml->token = yamllex(yaml->scanner);

        if(yaml->token == s_indent) {
            scope = yaml->space;
            yaml->token = yamllex(yaml->scanner);

            if(yaml_block(yaml, scope))
                status = panic("failed to block yaml object");
        } else {
            status = panic("expected space");
        }
    } else {
        status = panic("expected space or newline");
    }

    return status;
}

int yaml_scalar(struct yaml * yaml, int scope, int is_literal) {
    int status = 0;

    int newline;
    struct string * string;

    yaml->scalar = 1;

    if(yaml->token == b_break) {
        yaml->token = yamllex(yaml->scanner);
        while(yaml->token == l_empty)
            yaml->token = yamllex(yaml->scanner);

        if(yaml->token == s_indent) {
            if(scope < yaml->space) {
                scope = yaml->space;

                do {
                    if(strbuf_putcn(&yaml->strbuf, ' ', yaml->space - scope)) {
                        status = panic("failed to putcn strbuf object");
                    } else {
                        yaml->token = yamllex(yaml->scanner);

                        if(yaml->token == nb_char) {
                            if(strbuf_strcpy(&yaml->strbuf, yaml->string, yaml->length)) {
                                status = panic("failed to strcpy strbuf object");
                            } else {
                                yaml->token = yamllex(yaml->scanner);

                                if(yaml->token == b_break) {
                                    newline = 1;
                                    yaml->token = yamllex(yaml->scanner);
                                    while(yaml->token == l_empty) {
                                        newline++;
                                        yaml->token = yamllex(yaml->scanner);
                                    }

                                    if(is_literal) {
                                        if(strbuf_putcn(&yaml->strbuf, '\n', newline))
                                            status = panic("failed to putcn strbuf object");
                                    } else {
                                        if(newline == 1) {
                                            if(strbuf_putc(&yaml->strbuf, ' '))
                                                status = panic("failed to putcn strbuf object");
                                        } else {
                                            if(strbuf_putcn(&yaml->strbuf, '\n', newline - 1))
                                                status = panic("failed to putcn strbuf object");
                                        }
                                    }
                                } else {
                                    status = panic("expected newline");
                                }
                            }
                        } else {
                            status = panic("expected scalar");
                        }
                    }
                } while(yaml->token == s_indent && scope <= yaml->space && !status);

                if(!status) {
                    status = panic("failed to literal yaml object");
                } else {
                    string = strbuf_string(&yaml->strbuf);
                    if(!string) {
                        status = panic("failed to string strbuf object");
                    } else {
                        if(yaml->callback(event_scalar, string, yaml->context)) {
                            status = panic("failed to process scalar event");
                        } else {
                            strbuf_clear(&yaml->strbuf);
                        }
                    }
                }
            } else {
                status = panic("invalid scope");
            }
        } else {
            status = panic("expected space");
        }
    } else {
        status = panic("expected newline");
    }

    yaml->scalar = 0;

    return status;
}
