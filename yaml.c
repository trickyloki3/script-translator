#include "yaml.h"

#include "yaml_parser.h"
#include "yaml_scanner.h"

int yaml_node_create(struct yaml *, int, size_t, struct string *, struct yaml_node **);
void yaml_node_destroy(struct yaml *, struct yaml_node *);
void yaml_node_indent(struct yaml *, struct yaml_node *);
void yaml_node_print(struct yaml_node *);

int yaml_parse_loop(struct yaml *, yyscan_t, yamlpstate *);
void yaml_parse_reset(struct yaml *);

int yaml_node_create(struct yaml * yaml, int type, size_t scope, struct string * string, struct yaml_node ** result) {
    int status = 0;
    struct yaml_node * node;

    node = pool_get(yaml->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->type = type;
        node->scope = scope;
        node->string = string;
        node->child = NULL;
        *result = node;
    }

    return status;
}

void yaml_node_destroy(struct yaml * yaml, struct yaml_node * node) {
    pool_put(yaml->pool, node);
}

void yaml_node_indent(struct yaml * yaml, struct yaml_node * node) {
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

void yaml_node_print(struct yaml_node * node) {
    switch(node->type) {
        case yaml_c_sequence_entry: fprintf(stdout, "c_sequence_entry(%zu) ", node->scope); break;
        case yaml_c_mapping_key: fprintf(stdout, "c_mapping_key "); break;
        case yaml_c_mapping_value: fprintf(stdout, "c_mapping_value(%zu) ", node->scope); break;
        case yaml_c_literal: fprintf(stdout, "c_literal(%zu) ", node->scope); break;
        case yaml_c_folded: fprintf(stdout, "c_folded(%zu) ", node->scope); break;
        case yaml_s_indent: fprintf(stdout, "s_indent(%zu) ", node->scope); break;
        case yaml_s_separate_in_line: fprintf(stdout, "s_separate_in_line(%zu) ", node->scope); break;
        case yaml_l_empty: fprintf(stdout, "l_empty\n"); break;
        case yaml_b_break: fprintf(stdout, "b_break\n"); break;
        case yaml_nb_char: fprintf(stdout, "nb_char(%s) ", node->string->string); break;
        case yaml_ns_plain_one_line: fprintf(stdout, "ns_plain_one_line(%s) ", node->string->string); break;
    }
}

int yaml_create(struct yaml * yaml, size_t size, struct heap * heap) {
    int status = 0;

    yaml->pool = heap_pool(heap, sizeof(struct yaml_node));
    if(!yaml->pool) {
        status = panic("failed to pool heap object");
    } else if(strbuf_create(&yaml->strbuf, size)) {
        status = panic("failed to create strbuf object");
    } else {
        if(strbuf_create(&yaml->scalar, size)) {
            status = panic("failed to create strbuf object");
        } else {
            if(list_create(&yaml->list, heap->list_pool)) {
                status = panic("failed to create list object");
            } else {
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

    return status;
}

void yaml_destroy(struct yaml * yaml) {
    list_destroy(&yaml->list);
    strbuf_destroy(&yaml->scalar);
    strbuf_destroy(&yaml->strbuf);
}

int yaml_parse(struct yaml * yaml, const char * path, size_t size) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    yamlpstate * parser;
    YY_BUFFER_STATE buffer;

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

    yaml_parse_reset(yaml);

    return status;
}

void yaml_parse_reset(struct yaml * yaml) {
    struct yaml_node * node;

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

    node = list_pop(&yaml->list);
    while(node) {
        yaml_node_destroy(yaml, node);
        node = list_pop(&yaml->list);
    }

    strbuf_clear(&yaml->strbuf);
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
            yaml_node_indent(yaml, node);
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
        status = (yaml->stack->type == yaml_c_literal || yaml->stack->type == yaml_c_folded) && yaml->stack->scope < scope;
        /*
         * the scope of the block scalar is the same
         * as the scope of the  first non-empty line
         */
        if(status)
            yaml->stack->scope = scope;
    } else if(yaml->root) {
        status = (yaml->root->type == yaml_c_literal || yaml->root->type == yaml_c_folded) && yaml->root->scope <= scope;
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

int yaml_block(struct yaml * yaml, struct yaml_node * block) {
    int status = 0;
    struct yaml_node * node;
    struct yaml_node * list;
    struct yaml_node * peek;

    if(!yaml->root) {
        yaml->root = yaml->stack;
        yaml->stack = NULL;
    } else {
        while(yaml->root && yaml->root->scope > block->scope) {
            node = yaml->root;
            yaml->root = yaml->root->child;
            if(node->type == yaml_c_literal || node->type == yaml_c_folded)
                strbuf_clear(&yaml->scalar);
            yaml_node_destroy(yaml, node);
        }
        if(!yaml->root) {
            status = panic("invalid scope - %zu", block->scope);
        } else {
            switch(yaml->root->type) {
                case yaml_c_literal:
                case yaml_c_folded:
                    if(block->type != yaml_nb_char) {
                        status = panic("invalid scalar - %d", block->type);
                    } else if(strbuf_strcpy(&yaml->scalar, block->string->string, block->string->length)) {
                        status = panic("failed to strcpy strbuf object");
                    }
                    break;
                case yaml_c_sequence_entry:
                case yaml_c_mapping_value:
                    list = NULL;
                    while(yaml->stack) {
                        node = yaml->stack;
                        yaml->stack = yaml->stack->child;
                        node->child = list;
                        list = node;
                    }

                    if(list->type != yaml->root->type) {
                        status = panic("invalid sequence entry or map value");
                    } else if(list->scope != yaml->root->scope) {
                        status = panic("invalid scope - %d", list->scope);
                    } else {
                        node = list;
                        list = list->child;
                        yaml_node_destroy(yaml, node);

                        while(list) {
                            node = list;
                            list = list->child;
                            node->child = yaml->root;
                            yaml->root = node;
                        }
                    }

                    if(status) {
                        while(list) {
                            node = list;
                            list = list->child;
                            yaml_node_destroy(yaml, node);
                        }
                    }
                    break;
                default:
                    status = panic("invalid type - %d", yaml->root->type);
                    break;
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

        if(peek->string) {
            strbuf_clear_move(&yaml->strbuf, peek->string->string, peek->string->length);
            peek->string = strbuf_string(&yaml->strbuf);
            if(!peek->string) {
                status = panic("failed to string strbuf object");
            } else if(list_push(&yaml->list, peek)) {
                status = panic("failed to push list object");
            }
        } else {
            strbuf_clear(&yaml->strbuf);
            if(list_push(&yaml->list, peek))
                status = panic("failed to push list object");
        }

        if(status)
            yaml_node_destroy(yaml, peek);
    }

    return status;
}

void yaml_document(struct yaml * yaml) {
    struct yaml_node * node;

    while(yaml->root) {
        node = yaml->root;
        yaml->root = yaml->root->child;
        if(node->type == yaml_c_literal || node->type == yaml_c_folded)
            strbuf_clear(&yaml->scalar);
        yaml_node_destroy(yaml, node);
    }
}
