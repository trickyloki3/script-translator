#include "csv.h"

#include "csv_scanner.h"

int csv_parse(const char * path, csv_cb cb, void * arg) {
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
            csv.cb = cb;
            csv.arg = arg;

            if(csvlex(scanner))
                status = panic("failed to parse %s", path);

            csvlex_destroy(scanner);
        }
        fclose(file);
    }

    return status;
}
