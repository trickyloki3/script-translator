#include "json.h"

#include "json_parser.h"
#include "json_scanner.h"

int json_parse(const char * path, event_cb callback, void * context) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    struct json json;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(jsonlex_init_extra(&json, &scanner)) {
            status = panic("failed to create scanner object");
        } else {
            json.parser = jsonpstate_new();
            if(!json.parser) {
                status = panic("failed to create parser object");
            } else {
                jsonrestart(file, scanner);

                json.callback = callback;
                json.context = context;

                if(jsonlex(scanner))
                    status = panic("failed to parse %s", path);

                jsonpstate_delete(json.parser);
            }
            jsonlex_destroy(scanner);
        }
        fclose(file);
    }

    return status;
}
