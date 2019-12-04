#include "yaml.h"

#include "yaml_parser.h"
#include "yaml_scanner.h"

int yaml_parse_loop(struct yaml * yaml, yyscan_t, yamlpstate *);

int yaml_create(struct yaml * yaml, struct heap * heap) {
    int status = 0;

    return status;
}

void yaml_destroy(struct yaml * yaml) {

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
