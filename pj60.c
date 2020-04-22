#include "parser.h"

int main(int argc, char ** argv) {
    int status = 0;
    struct heap heap;
    struct schema schema;
    struct parser parser;

    if(argc < 2) {
        status = panic("%s <path> <name>", argv[0]);
    } else if(heap_create(&heap, 4096)) {
        status = panic("failed to create heap object");
    } else {
        if(schema_create(&schema, 4096)) {
            status = panic("failed to create schema object");
        } else {
            if(parser_create(&parser, 4096, &heap)) {
                status = panic("failed to create parser object");
            } else {
                if(parser_schema_parse(&parser, &schema, argv[1])) {
                    status = panic("failed to schema parse parser object");
                } else {
                    schema_print(&schema);
                }
                parser_destroy(&parser);
            }
            schema_destroy(&schema);
        }
        heap_destroy(&heap);
    }

    return status;
}
