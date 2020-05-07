#include "yaml.h"

#include "yaml_scanner.h"

typedef int (* yaml_cb)(struct yaml *);

int yaml_start(struct yaml *, enum yaml_type);
int yaml_next(struct yaml *);
int yaml_end(struct yaml *, enum yaml_type);

int yaml_push(struct yaml *, enum yaml_type);
int yaml_pop(struct yaml *, int);

int yaml_document(struct yaml *);
int yaml_block(struct yaml *);
int yaml_plain(struct yaml *);
int yaml_container(struct yaml *, yaml_cb);
int yaml_scalar(struct yaml *, yaml_cb);
int yaml_literal(struct yaml *);
int yaml_folded(struct yaml *);

int yaml_create(struct yaml * yaml, size_t size, struct heap * heap) {
    int status = 0;

    yaml->pool = heap_pool(heap, sizeof(struct yaml_node));
    if(!yaml->pool) {
        status = panic("failed to pool heap object");
    } else {
        if(strbuf_create(&yaml->strbuf, size)) {
            status = panic("failed to create strbuf object");
        } else {
            if(yamllex_init_extra(yaml, &yaml->scanner))
                status = panic("failed to create scanner object");
            if(status)
                strbuf_destroy(&yaml->strbuf);
        }
    }

    return status;
}

void yaml_destroy(struct yaml * yaml) {
    yamllex_destroy(yaml->scanner);
    strbuf_destroy(&yaml->strbuf);
}

int yaml_parse(struct yaml * yaml, const char * path, event_cb callback, void * context) {
    int status = 0;

    FILE * file;
    struct yaml_node * node;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        yaml->root = NULL;
        strbuf_clear(&yaml->strbuf);
        yamlrestart(file, yaml->scanner);
        yaml->string = NULL;
        yaml->length = 0;
        yaml->space = 0;
        yaml->token = 0;
        yaml->scope = 0;
        yaml->scalar = 0;
        yaml->callback = callback;
        yaml->context = context;

        if(yaml_document(yaml)) {
            status = panic("failed to document yaml object");
        } else if(yaml_pop(yaml, -1)) {
            status = panic("failed to pop yaml object");
        }

        while(yaml->root) {
            node = yaml->root;
            yaml->root = yaml->root->next;
            pool_put(yaml->pool, node);
        }

        fclose(file);
    }

    return status;
}

int yaml_start(struct yaml * yaml, enum yaml_type type) {
    int status = 0;

    if(type == yaml_sequence) {
        if(yaml->callback(event_list_start, NULL, yaml->context))
            status = panic("failed to process list start event");
    } else if(type == yaml_map) {
        if(yaml->callback(event_map_start, NULL, yaml->context))
            status = panic("failed to process map start event");
    } else {
        status = panic("invalid type - %d", type);
    }

    return status;
}

int yaml_next(struct yaml * yaml) {
    int status = 0;
    struct string * string;

    string = strbuf_string(&yaml->strbuf);
    if(!string) {
        status = panic("failed to string strbuf object");
    } else if(yaml->callback(event_scalar, string, yaml->context)) {
        status = panic("failed to process scalar event");
    }

    strbuf_clear(&yaml->strbuf);

    return status;
}

int yaml_end(struct yaml * yaml, enum yaml_type type) {
    int status = 0;

    if(type == yaml_sequence) {
        if(yaml->callback(event_list_end, NULL, yaml->context))
            status = panic("failed to process list end event");
    } else if(type == yaml_map) {
        if(yaml->callback(event_map_end, NULL, yaml->context))
            status = panic("failed to process map end event");
    } else {
        status = panic("invalid type - %d", type);
    }

    return status;
}

int yaml_push(struct yaml * yaml, enum yaml_type type) {
    int status = 0;
    struct yaml_node * node;

    if(yaml->root && yaml->root->scope >= yaml->scope) {
        status = panic("invalid scope");
    } else {
        node = pool_get(yaml->pool);
        if(!node) {
            status = panic("out of memory");
        } else {
            node->type = type;
            node->scope = yaml->scope;
            node->next = yaml->root;
            yaml->root = node;

            if(yaml_start(yaml, yaml->root->type))
                status = panic("failed to start yaml object");
        }
    }

    return status;
}

int yaml_pop(struct yaml * yaml, int scope) {
    int status = 0;
    struct yaml_node * node;

    while(yaml->root && yaml->root->scope > scope && !status) {
        if(yaml_end(yaml, yaml->root->type)) {
            status = panic("failed to end yaml object");
        } else {
            node = yaml->root;
            yaml->root = yaml->root->next;
            pool_put(yaml->pool, node);
        }
    }

    return status;
}

int yaml_document(struct yaml * yaml) {
    int status = 0;

    yaml->token = yamllex(yaml->scanner);
    while(yaml->token == l_empty)
        yaml->token = yamllex(yaml->scanner);

    while(yaml->token && !status) {
        yaml->scope = 0;

        if(yaml->token == s_indent) {
            yaml->scope = yaml->space;
            yaml->token = yamllex(yaml->scanner);
        }

        if(yaml_pop(yaml, yaml->scope)) {
            status = panic("failed to pop yaml object");
        } else {
            if(yaml->root) {
                if(yaml->root->scope != yaml->scope) {
                    status = panic("invalid scope");
                } else if(yaml->root->type == yaml_sequence) {
                    if(yaml->token == c_sequence_entry) {
                        yaml->token = yamllex(yaml->scanner);

                        if(yaml_container(yaml, yaml_block))
                            status = panic("failed to container yaml object");
                    } else {
                        status = panic("expected sequence entry");
                    }
                } else if(yaml->root->type == yaml_map) {
                    if(yaml->token == ns_plain_one_line) {
                        if(strbuf_strcpy(&yaml->strbuf, yaml->string, yaml->length)) {
                            status = panic("failed to strcpy strbuf object");
                        } else {
                            yaml->token = yamllex(yaml->scanner);

                            if(yaml->token == c_mapping_value) {
                                yaml->token = yamllex(yaml->scanner);

                                if(yaml_next(yaml)) {
                                    status = panic("failed to next yaml object");
                                } else if(yaml_container(yaml, yaml_plain)) {
                                    status = panic("failed to container yaml object");
                                }
                            } else {
                                status = panic("expected mapping value");
                            }
                        }
                    } else {
                        status = panic("expected mapping key");
                    }
                } else {
                    status = panic("invalid type - %d", yaml->root->type);
                }
            } else if(yaml_block(yaml)) {
                status = panic("failed to block yaml object");
            }
        }
    }

    return status;
}

int yaml_block(struct yaml * yaml) {
    int status = 0;

    if(yaml->token == ns_plain_one_line) {
        if(strbuf_strcpy(&yaml->strbuf, yaml->string, yaml->length)) {
            status = panic("failed to strcpy strbuf object");
        } else {
            yaml->token = yamllex(yaml->scanner);

            if(yaml->token == b_break) {
                yaml->token = yamllex(yaml->scanner);
                while(yaml->token == l_empty)
                    yaml->token = yamllex(yaml->scanner);

                if(yaml_next(yaml))
                    status = panic("failed to next yaml object");
            } else if(yaml->token == c_mapping_value) {
                yaml->token = yamllex(yaml->scanner);

                if(yaml_push(yaml, yaml_map)) {
                    status = panic("failed to start yaml object");
                } else if(yaml_next(yaml)) {
                    status = panic("failed to next yaml object");
                } else if(yaml_container(yaml, yaml_plain)) {
                    status = panic("failed to container yaml object");
                }
            } else {
                status = panic("expected newline or mapping value");
            }
        }
    } else if(yaml->token == c_sequence_entry) {
        yaml->token = yamllex(yaml->scanner);
        if(yaml_push(yaml, yaml_sequence)) {
            status = panic("failed ot start yaml object");
        } else if(yaml_container(yaml, yaml_block)) {
            status = panic("failed to container yaml object");
        }
    } else if(yaml_plain(yaml)) {
        status = panic("failed to plain yaml object");
    }

    return status;
}

int yaml_plain(struct yaml * yaml) {
    int status = 0;

    if(yaml->token == ns_plain_one_line) {
        if(strbuf_strcpy(&yaml->strbuf, yaml->string, yaml->length)) {
            status = panic("failed to strcpy strbuf object");
        } else {
            yaml->token = yamllex(yaml->scanner);

            if(yaml->token == b_break) {
                yaml->token = yamllex(yaml->scanner);
                while(yaml->token == l_empty)
                    yaml->token = yamllex(yaml->scanner);

                if(yaml_next(yaml))
                    status = panic("failed to next yaml object");
            } else {
                status = panic("expected newline");
            }
        }
    } else if(yaml->token == c_literal) {
        yaml->token = yamllex(yaml->scanner);
        if(yaml_scalar(yaml, yaml_literal)) {
            status = panic("failed to scalar yaml object");
        } else if(yaml_next(yaml)) {
            status = panic("failed to next yaml object");
        }
    } else if(yaml->token == c_folded) {
        yaml->token = yamllex(yaml->scanner);
        if(yaml_scalar(yaml, yaml_folded)) {
            status = panic("failed to scalar yaml object");
        } else if(yaml_next(yaml)) {
            status = panic("failed to next yaml object");
        }
    } else {
        status = panic("invalid token - %d", yaml->token);
    }

    return status;
}

int yaml_container(struct yaml * yaml, yaml_cb callback) {
    int status = 0;

    if(yaml->token == s_separate_in_line) {
        yaml->scope += yaml->space + 1;
        yaml->token = yamllex(yaml->scanner);

        if(callback(yaml))
            status = panic("failed to block yaml object");
    } else if(yaml->token == b_break) {
        yaml->token = yamllex(yaml->scanner);
        while(yaml->token == l_empty)
            yaml->token = yamllex(yaml->scanner);

        if(yaml->token == s_indent) {
            yaml->scope = yaml->space;
            yaml->token = yamllex(yaml->scanner);

            if(yaml_block(yaml))
                status = panic("failed to block yaml object");
        } else {
            status = panic("expected space");
        }
    } else {
        status = panic("expected space or newline");
    }

    return status;
}

int yaml_scalar(struct yaml * yaml, yaml_cb callback) {
    int status = 0;

    yaml->scalar = 1;

    if(yaml->token == b_break) {
        yaml->token = yamllex(yaml->scanner);
        while(yaml->token == l_empty)
            yaml->token = yamllex(yaml->scanner);

        if(yaml->token == s_indent) {
            if(!yaml->root || yaml->root->scope < yaml->space) {
                yaml->scope = yaml->space;

                if(callback(yaml))
                    status = panic("failed to scalar yaml object");
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

int yaml_literal(struct yaml * yaml) {
    int status = 0;
    int newline = 0;

    while(!status && yaml->token == s_indent && yaml->scope <= yaml->space) {
        if(strbuf_putcn(&yaml->strbuf, '\n', newline)) {
            status = panic("failed to putcn strbuf object");
        } else if(strbuf_putcn(&yaml->strbuf, ' ', yaml->space - yaml->scope)) {
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
                    } else {
                        status = panic("expected newline");
                    }
                }
            } else {
                status = panic("expected scalar");
            }
        }
    }

    if(!status && strbuf_putc(&yaml->strbuf, '\n'))
        status = panic("failed to putcn strbuf object");

    return status;
}

int yaml_folded(struct yaml * yaml) {
    int status = 0;
    int space = 0;
    int newline = 0;

    while(!status && yaml->token == s_indent && yaml->scope <= yaml->space) {
        if(newline == 1) {
            if(strbuf_putc(&yaml->strbuf, space || yaml->scope < yaml->space ? '\n' : ' '))
                status = panic("failed to putcn strbuf object");
        } else if(newline) {
            if(strbuf_putcn(&yaml->strbuf, '\n', newline - 1))
                status = panic("failed to putcn strbuf object");
        }

        if(!status) {
            space = yaml->space - yaml->scope;

            if(strbuf_putcn(&yaml->strbuf, ' ', space)) {
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
                        } else {
                            status = panic("expected newline");
                        }
                    }
                } else {
                    status = panic("expected scalar");
                }
            }
        }
    }

    return status;
}
