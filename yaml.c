#include "yaml.h"

#include "yaml_parser.h"
#include "yaml_scanner.h"

int yaml_ptr_compare(void *, void *);

int yaml_node_create(struct yaml *, enum yaml_type, char *, size_t, struct yaml_node **);
void yaml_node_destroy(struct yaml *, struct yaml_node *);

int yaml_parse_loop(struct yaml * yaml, yyscan_t, yamlpstate *);
void yaml_token_print(int);

int yaml_ptr_compare(void * x, void * y) {
    uintptr_t l = (uintptr_t) x;
    uintptr_t r = (uintptr_t) y;
    return l < r ? -1 : l > r ? 1 : 0;
}

int yaml_node_create(struct yaml * yaml, enum yaml_type type, char * string, size_t length, struct yaml_node ** result) {
    int status = 0;
    struct yaml_node * node;

    node = pool_get(yaml->yaml_node_pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->type = type;
        switch(node->type) {
            case yaml_sequence:
                if(list_create(&node->sequence, yaml->list_node_pool))
                    status = panic("failed to create list object");
                break;
            case yaml_mapping:
                if(map_create(&node->mapping, yaml_ptr_compare, yaml->map_node_pool))
                    status = panic("failed to create map object");
                break;
            case yaml_scalar:
                if(sstring_create(&node->scalar, string, length, yaml->sector_list))
                    status = panic("failed to create string object");
                break;
            default:
                status = panic("invalid type - %d", node->type);
                break;
        }
        if(status)
            pool_put(yaml->yaml_node_pool, node);
    }

    return status;
}

void yaml_node_destroy(struct yaml * yaml, struct yaml_node * node) {
    switch(node->type) {
        case yaml_sequence:
            list_destroy(&node->sequence);
            break;
        case yaml_mapping:
            map_destroy(&node->mapping);
            break;
        case yaml_scalar:
            sstring_destroy(node->scalar);
            break;
    }
    pool_put(yaml->yaml_node_pool, node);
}

int yaml_create(struct yaml * yaml, struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;

    yaml->list_node_pool = pool_map_get(pool_map, sizeof(struct list_node));
    if(!yaml->list_node_pool) {
        status = panic("failed to get pool map object");
    } else {
        yaml->map_node_pool = pool_map_get(pool_map, sizeof(struct map_node));
        if(!yaml->map_node_pool) {
            status = panic("failed to get pool map object");
        } else {
            yaml->yaml_node_pool = pool_map_get(pool_map, sizeof(struct yaml_node));
            if(!yaml->yaml_node_pool) {
                status = panic("failed to get pool map object");
            } else {
                if(list_create(&yaml->list, yaml->list_node_pool)) {
                    status = panic("failed to create list object");
                } else {
                    if(list_create(&yaml->nest, yaml->list_node_pool)) {
                        status = panic("failed to create list object");
                    } else {
                        yaml->root = NULL;
                        yaml->sector_list = sector_list;
                    }
                    if(status)
                        list_destroy(&yaml->list);
                }
            }
        }
    }

    return status;
}

void yaml_destroy(struct yaml * yaml) {
    yaml_clear(yaml);
    list_destroy(&yaml->nest);
    list_destroy(&yaml->list);
}

void yaml_clear(struct yaml * yaml) {
    struct yaml_node * node;

    node = list_pop(&yaml->list);
    while(node) {
        yaml_node_destroy(yaml, node);
        node = list_pop(&yaml->list);
    }
    yaml->root = NULL;
}

int yaml_parse(struct yaml * yaml, const char * path) {
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
                buffer = yaml_create_buffer(file, 4096, scanner);
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

    return status;
}

void yaml_token_print(int token) {
    switch(token) {
        case yaml_c_sequence_entry:     fprintf(stderr, "c_sequence_entry "); break;
        case yaml_c_mapping_key:        fprintf(stderr, "c_mapping_key "); break;
        case yaml_c_mapping_value:      fprintf(stderr, "c_mapping_value "); break;
        case yaml_s_indent:             fprintf(stderr, "s_indent "); break;
        case yaml_s_separate_in_line:   fprintf(stderr, "s_separate_in_line "); break;
        case yaml_b_break:              fprintf(stderr, "b_break\n"); break;
        case yaml_l_empty:              fprintf(stderr, "l_empty "); break;
        case yaml_ns_yaml_directive:    fprintf(stderr, "ns_yaml_directive "); break;
        case yaml_ns_tag_directive:     fprintf(stderr, "ns_tag_directive "); break;
        case yaml_ns_reserve_directive: fprintf(stderr, "ns_reserve_directive "); break;
        default:                        fprintf(stderr, "<%d> ", token); break;
    }
}
