#include "json.h"

#include "json_parser.h"
#include "json_scanner.h"

int json_parse_loop(yyscan_t, jsonpstate *);

int json_parse(const char * path) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    jsonpstate * parser;
    YY_BUFFER_STATE buffer;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(jsonlex_init_extra(NULL, &scanner)) {
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
                    if(json_parse_loop(scanner, parser))
                        status = panic("failed to parse loop json object");
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

int json_parse_loop(yyscan_t scanner, jsonpstate * parser) {
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
            state = jsonpush_parse(parser, token, &value, &location);
            if(state && state != YYPUSH_MORE)
                status = panic("failed to parse the current token");
        }
    } while(token && state == YYPUSH_MORE && !status);

    return status;
}
