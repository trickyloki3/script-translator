#include "json.h"

#include "json_parser.h"
#include "json_scanner.h"

int json_parse_loop(struct json *, yyscan_t, jsonpstate *);

int json_create(struct json * json, size_t size) {
    int status = 0;

    if(strbuf_create(&json->strbuf, size)) {
        status = panic("failed to create strbuf object");
    } else {
        json->token = 0;
        json->string = NULL;
    }

    return status;
}

void json_destroy(struct json * json) {
    strbuf_destroy(&json->strbuf);
}

int json_parse(struct json * json, const char * path, size_t size, event_cb callback, void * context) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    jsonpstate * parser;
    YY_BUFFER_STATE buffer;

    json->callback = callback;
    json->context = context;

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
                buffer = json_create_buffer(file, size, scanner);
                if(!buffer) {
                    status = panic("failed to create buffer state object");
                } else {
                    jsonpush_buffer_state(buffer, scanner);
                    if(json_parse_loop(json, scanner, parser))
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
            state = jsonpush_parse(parser, token, &value, &location);
            if(state && state != YYPUSH_MORE)
                status = panic("failed to parse the current token");
        }
    } while(token && state == YYPUSH_MORE && !status);

    if(status) {
        /* skip the last token on error */
    } else if(json_token(json, 0, NULL, 0)) {
        status = panic("failed to token json object");
    }

    /*
     * reset json object to initial state
     */
    json->string = NULL;
    json->token = 0;
    strbuf_clear(&json->strbuf);

    return status;
}

int json_token(struct json * json, int token, char * string, size_t length) {
    int status = 0;

    /*
     * events are generated from scanner, but
     * takes into account the lookahead token
     */
    if(json->token) {
        if(json->callback(json->token, json->string, json->context))
            status = panic("failed to process event");
        strbuf_clear(&json->strbuf);
    }

    if(status) {
        /* skip the next token on error */
    } else {
        json->token = token;
        if(string) {
            if(strbuf_strcpy(&json->strbuf, string, length)) {
                status = panic("failed to strcpy strbuf object");
            } else {
                json->string = strbuf_string(&json->strbuf);
                if(!json->string)
                    status = panic("failed to string strbuf object");
            }
        } else {
            json->string = NULL;
        }
    }

    return status;
}
