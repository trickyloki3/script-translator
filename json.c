#include "json.h"

#include "json_parser.h"
#include "json_scanner.h"

int json_string_compare(void *, void *);

int json_parse_loop(struct json *, yyscan_t, jsonpstate *);

int json_node_create(struct json_node *, enum json_type type, struct pool *, struct pool *, struct sector_list *, char *, size_t);
void json_node_destroy(struct json_node *);
void json_node_print_recursive(struct json_node *, int);

int json_string_compare(void * x, void * y) {
    return strcmp(x, y);
}

int json_create(struct json * json, struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;

    json->map_node_pool = pool_map_get(pool_map, sizeof(struct map_node));
    if(!json->map_node_pool) {
        status = panic("failed to get pool map object");
    } else {
        json->list_node_pool = pool_map_get(pool_map, sizeof(struct list_node));
        if(!json->list_node_pool) {
            status = panic("failed to get pool map object");
        } else {
            json->json_node_pool = pool_map_get(pool_map, sizeof(struct json_node));
            if(!json->json_node_pool) {
                status = panic("failed to get pool map object");
            } else {
                if(list_create(&json->list, json->list_node_pool)) {
                    status = panic("failed to create list object");
                } else {
                    if(list_create(&json->nest, json->list_node_pool)) {
                        status = panic("failed to create list object");
                    } else {
                        json->root = NULL;
                        json->sector_list = sector_list;
                    }
                    if(status)
                        list_destroy(&json->list);
                }
            }
        }
    }

    return status;
}

void json_destroy(struct json * json) {
    json_clear(json);
    list_destroy(&json->nest);
    list_destroy(&json->list);
}

void json_clear(struct json * json) {
    struct json_node * node;

    node = list_pop(&json->list);
    while(node) {
        json_node_destroy(node);
        pool_put(json->json_node_pool, node);
        node = list_pop(&json->list);
    }
    json->root = NULL;
}

int json_parse(struct json * json, const char * path, struct json_node ** result) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    jsonpstate * parser;
    YY_BUFFER_STATE buffer;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(jsonlex_init_extra(json, &scanner)) {
            status = panic("failed to create scanner object");
        } else {
            parser = jsonpstate_new();
            if(!parser) {
                status = panic("failed to create parser object");
            } else {
                buffer = json_create_buffer(file, 4096, scanner);
                if(!buffer) {
                    status = panic("faield to create buffer state object");
                } else {
                    jsonpush_buffer_state(buffer, scanner);
                    if(json_parse_loop(json, scanner, parser)) {
                        status = panic("failed to parse loop json object");
                    } else {
                        *result = json->root;
                    }
                    jsonpop_buffer_state(scanner);
                }
                jsonpstate_delete(parser);
            }
            jsonlex_destroy(scanner);
        }
        fclose(file);
    }

    return status;
}

int json_parse_loop(struct json * json, yyscan_t scanner, jsonpstate * parser) {
    int status = 0;

    JSONSTYPE value;
    JSONLTYPE location;
    int token;
    int state;

    do {
        token = jsonlex(&value, &location, scanner);
        if(token < 0) {
            status = panic("failed to get the next token");
        } else {
            state = jsonpush_parse(parser, token, &value, &location, json);
            if(state && state != YYPUSH_MORE)
                status = panic("failed to parse the current token");
        }
    } while(token && state == YYPUSH_MORE && !status);

    if(!status) {
        if(list_start(&json->nest)) {
            status = panic("nest is not empty");
        } else if(!json->root) {
            status = panic("json is empty");
        }
    }

    return status;
}

int json_node_create(struct json_node * node, enum json_type type, struct pool * map_node_pool, struct pool * list_node_pool, struct sector_list * sector_list, char * string, size_t length) {
    int status = 0;
    char * end;

    node->type = type;
    switch(node->type) {
        case json_false:
        case json_null:
        case json_true:
            break;
        case json_object:
            if(map_create(&node->map, json_string_compare, map_node_pool))
                status = panic("failed to create map object");
            break;
        case json_array:
            if(list_create(&node->list, list_node_pool))
                status = panic("failed to create list object");
            break;
        case json_number:
            node->number = strtod(string, &end);
            if(*end)
                status = panic("invalid string '%s' in '%s'", end, string);
            break;
        case json_string:
            if(sstring_create(&node->string, string, length, sector_list))
                status = panic("failed to create sstring object");
            break;
        default:
            status = panic("invalid type - %d", node->type);
            break;
    }

    return status;
}

void json_node_destroy(struct json_node * node) {
    switch(node->type) {
        case json_object:
            map_destroy(&node->map);
            break;
        case json_array:
            list_destroy(&node->list);
            break;
        case json_string:
            sstring_destroy(node->string);
            break;
        default:
            break;
    }
}

int json_add_node(struct json * json, enum json_type type, char * string, size_t length, struct json_node ** result) {
    int status = 0;
    struct json_node * node;

    node = pool_get(json->json_node_pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        if(json_node_create(node, type, json->map_node_pool, json->list_node_pool, json->sector_list, string, length)) {
            status = panic("failed to create json node object");
        } else {
            if(list_push(&json->list, node)) {
                status = panic("failed to push list object");
            } else {
                *result = node;
            }
            if(status)
                json_node_destroy(node);
        }
        if(status)
            pool_put(json->json_node_pool, node);
    }


    return status;
}

int json_push_node(struct json * json, struct json_node * node) {
    int status = 0;

    if(node->type != json_object && node->type != json_array) {
        status = panic("invalid type - %d", node->type);
    } else if(list_push(&json->nest, node)) {
        status = panic("failed to push list object");
    }

    return status;
}

int json_pop_node(struct json * json, enum json_type type) {
    int status = 0;
    struct json_node * node;

    node = list_pop(&json->nest);
    if(!node) {
        status = panic("list is empty");
    } else if(node->type != type) {
        status = panic("invalid type - %d", node->type);
    }

    return status;
}

int json_insert_object(struct json * json, struct json_node * string, struct json_node * value) {
    int status = 0;
    struct json_node * node;

    node = list_start(&json->nest);
    if(!node) {
        status = panic("list is empty");
    } else if(node->type != json_object) {
        status = panic("invalid object");
    } else if(map_insert(&node->map, string->string, value)) {
        status = panic("failed to insert map object");
    }

    return status;
}

int json_insert_array(struct json * json, struct json_node * value) {
    int status = 0;
    struct json_node * node;

    node = list_start(&json->nest);
    if(!node) {
        status = panic("list is empty");
    } else if(node->type != json_array) {
        status = panic("invalid array");
    } else if(list_push(&node->list, value)) {
        status = panic("failed to push list object");
    }

    return status;
}

void json_node_print(struct json_node * node) {
    json_node_print_recursive(node, 1);
}

void json_node_print_recursive(struct json_node * node, int indent) {
    int i;
    struct json_node * iter;
    struct map_pair pair;

    if(node) {
        switch(node->type) {
            case json_false:  fprintf(stdout, "false"); break;
            case json_null:   fprintf(stdout, "null"); break;
            case json_true:   fprintf(stdout, "true"); break;
            case json_object:
                fprintf(stdout, "{\n");
                pair = map_start(&node->map);
                while(pair.key && pair.value) {
                    for(i = 0; i < indent; i++)
                        fputs("    ", stdout);
                    fprintf(stdout, "\"%s\" : ", (sstring) pair.key);
                    json_node_print_recursive(pair.value, indent + 1);
                    pair = map_next(&node->map);
                    if(pair.key && pair.value)
                        fprintf(stdout, ",\n");
                    else
                        fprintf(stdout, "\n");
                }
                for(i = 0; i < indent - 1; i++)
                    fputs("    ", stdout);
                fprintf(stdout, "}");
                break;
            case json_array:
                fprintf(stdout, "[\n");
                iter = list_start(&node->list);
                while(iter) {
                    for(i = 0; i < indent; i++)
                        fputs("    ", stdout);
                    json_node_print_recursive(iter, indent + 1);
                    iter = list_next(&node->list);
                    if(iter)
                        fprintf(stdout, ",\n");
                    else
                        fprintf(stdout, "\n");
                }
                for(i = 0; i < indent - 1; i++)
                    fputs("    ", stdout);
                fprintf(stdout, "]");
                break;
            case json_number: fprintf(stdout, "%ld", (long) node->number); break;
            case json_string: fprintf(stdout, "\"%s\"", node->string); break;
            default:
                break;
        }
    }
}

struct json_node * json_object_get(struct json_node * node, char * key) {
    return node->type == json_object ? map_search(&node->map, key) : NULL;
}

struct map_pair json_object_start(struct json_node * node) {
    struct map_pair zero = { NULL, NULL };
    return node->type == json_object ? map_start(&node->map) : zero;
}

struct map_pair json_object_next(struct json_node * node) {
    struct map_pair zero = { NULL, NULL };
    return node->type == json_object ? map_next(&node->map) : zero;
}

struct json_node * json_array_start(struct json_node * node) {
    return node->type == json_array ? list_start(&node->list) : NULL;
}

struct json_node * json_array_next(struct json_node * node) {
    return node->type == json_array ? list_next(&node->list) : NULL;
}

sstring json_string_get(struct json_node * node) {
    return node->type == json_string ? node->string : NULL;
}

double json_number_get(struct json_node * node) {
    return node->type == json_number ? node->number : 0;
}
