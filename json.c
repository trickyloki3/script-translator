#include "json.h"

#include "json_parser.h"
#include "json_scanner.h"

int json_parse_loop(struct json *, yyscan_t, jsonpstate *);

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
    list_destroy(&json->nest);
    list_destroy(&json->list);
}

int json_parse(struct json * json, const char * path) {
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
                    status = json_parse_loop(json, scanner, parser);
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

    return status;
}
