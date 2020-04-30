#include "csv.h"

#include "csv_scanner.h"

int csv_parse(const char * path, parser_cb callback, void * context) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    YY_BUFFER_STATE buffer;
    struct csv csv;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(csvlex_init_extra(&csv, &scanner)) {
            status = panic("failed to create scanner object");
        } else {
            buffer = csv_create_buffer(file, 4096, scanner);
            if(!buffer) {
                status = panic("failed to create buffer state object");
            } else {
                csvpush_buffer_state(buffer, scanner);

                csv.index = 0;
                csv.callback = callback;
                csv.context = context;

                if(csvlex(scanner))
                    status = panic("failed to scan csv object");

                csvpop_buffer_state(scanner);
            }
            csvlex_destroy(scanner);
        }
        fclose(file);
    }

    return status;
}
