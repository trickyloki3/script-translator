#include "csv.h"

#include "csv_scanner.h"

int csv_parse(const char * path, parser_cb callback, void * context) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    struct csv csv;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(csvlex_init_extra(&csv, &scanner)) {
            status = panic("failed to create scanner object");
        } else {
            csvrestart(file, scanner);

            csv.index = 0;
            csv.callback = callback;
            csv.context = context;

            if(csvlex(scanner))
                status = panic("failed to parse %s", path);

            csvlex_destroy(scanner);
        }
        fclose(file);
    }

    return status;
}
