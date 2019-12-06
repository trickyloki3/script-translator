#include "yaml.h"

#include "yaml_parser.h"
#include "yaml_scanner.h"

int yaml_node_create(struct yaml *, int, size_t, struct string *, struct yaml_node **);
void yaml_node_destroy(struct yaml *, struct yaml_node *);
void yaml_node_indent(struct yaml *, struct yaml_node *);

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

int yaml_create(struct yaml * yaml, size_t size, struct heap * heap) {
    int status = 0;

    yaml->pool = heap_pool(heap, sizeof(struct yaml_node));
    if(!yaml->pool) {
        status = panic("failed to pool heap object");
    } else if(strbuf_create(&yaml->strbuf, size)) {
        status = panic("failed to create strbuf object");
    } else {
        if(list_create(&yaml->list, heap->list_pool)) {
            status = panic("failed to create list object");
        } else {
            yaml->root = NULL;
            yaml->indent = NULL;
        }
        if(status)
            strbuf_destroy(&yaml->strbuf);
    }

    return status;
}

void yaml_destroy(struct yaml * yaml) {
    list_destroy(&yaml->list);
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
    yaml->root = NULL;

    node = list_pop(&yaml->list);
    while(node) {
        yaml_node_destroy(yaml, node);
        node = list_pop(&yaml->list);
    }

    strbuf_clear(&yaml->strbuf);
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

int yaml_block(struct yaml * yaml, struct yaml_node * scope, struct yaml_node * block) {
    int status = 0;

    return status;
}
