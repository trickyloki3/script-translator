#include "csv.h"

#include "csv_parser.h"
#include "csv_scanner.h"

int csv_parse_loop(struct csv *, yyscan_t, csvpstate *);

int csv_create(struct csv * csv, size_t size, struct heap * heap) {
    int status = 0;

    if(strbuf_create(&csv->strbuf, size)) {
        status = panic("failed to create strbuf object");
    } else {
        if(list_create(&csv->list, heap->list_pool))
            status = panic("failed to create list object");
        if(status)
            strbuf_destroy(&csv->strbuf);
    }

    return status;
}

void csv_destroy(struct csv * csv) {
    list_destroy(&csv->list);
    strbuf_destroy(&csv->strbuf);
}

int csv_parse(struct csv * csv, const char * path, size_t size) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    csvpstate * parser;
    YY_BUFFER_STATE buffer;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        if(csvlex_init_extra(&csv->strbuf, &scanner)) {
            status = panic("failed to create scanner object");
        } else {
            parser = csvpstate_new();
            if(!parser) {
                status = panic("failed to create parser object");
            } else {
                buffer = csv_create_buffer(file, size, scanner);
                if(!buffer) {
                    status = panic("failed to create buffer state object");
                } else {
                    csvpush_buffer_state(buffer, scanner);
                    status = csv_parse_loop(csv, scanner, parser);
                    csvpop_buffer_state(scanner);
                }
                csvpstate_delete(parser);
            }
            csvlex_destroy(scanner);
        }
        fclose(file);
    }

    return status;
}

int csv_parse_loop(struct csv * csv, yyscan_t scanner, csvpstate * parser) {
    int status = 0;

    CSVSTYPE value;
    CSVLTYPE location;
    int token;
    int state;

    do {
        token = csvlex(&value, &location, scanner);
        if(token < 0) {
            status = panic("failed to get the next token");
        } else {
            state = csvpush_parse(parser, token, &value, &location, csv);
            if(state && state != YYPUSH_MORE)
                status = panic("failed to parse the current token");
        }
    } while(token && state == YYPUSH_MORE && !status);

    csv_reset(csv);

    return status;
}

int csv_push(struct csv * csv) {
    int status = 0;
    struct string * string;

    string = strbuf_string(&csv->strbuf);
    if(!string) {
        status = panic("failed to string strbuf object");
    } else if(list_push(&csv->list, string)) {
        status = panic("failed to push list object");
    }

    return status;
}

int csv_pop(struct csv * csv) {
    int status = 0;
    struct string * string;

    if(csv->list.size) {
        string = list_start(&csv->list);
        while(string) {

            string = list_next(&csv->list);
        }
    }
    csv_reset(csv);

    return status;
}

void csv_reset(struct csv * csv) {
    list_clear(&csv->list);
    strbuf_clear(&csv->strbuf);
}
