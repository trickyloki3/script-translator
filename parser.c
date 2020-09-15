#include "parser.h"

int parser_create(struct parser * parser, size_t size) {
    int status = 0;

    if(yaml_create(&parser->yaml, 64, size)) {
        status = panic("failed to create yaml object");
    } else {
        if(meta_create(&parser->meta, size))
            status = panic("failed to create meta object");
        if(status)
            yaml_destroy(&parser->yaml);
    }

    return status;
}

void parser_destroy(struct parser * parser) {
    meta_destroy(&parser->meta);
    yaml_destroy(&parser->yaml);
}

int parser_file(struct parser * parser, struct tag * tag, const char * path, yaml_reader_cb cb, void * arg) {
    if(meta_load(&parser->meta, tag))
        return panic("failed to load meta object");

    if(yaml_reader_parse(&parser->yaml, path, &parser->meta, cb, arg))
        return panic("failed to parse yaml object");

    return 0;
}
