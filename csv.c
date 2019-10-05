#include "csv.h"

#include "csv_parser.h"
#include "csv_scanner.h"

int csv_parse_loop(struct csv *, yyscan_t, csvpstate *);

int csv_create(struct csv * csv, size_t buffer_size, struct pool * list_node_pool) {
    int status = 0;

    if(list_create(&csv->string, list_node_pool)) {
        status = panic("failed to create list object");
    } else {
        if(list_create(&csv->active, list_node_pool)) {
            status = panic("failed to create list object");
        } else {
            if(list_create(&csv->record, list_node_pool)) {
                status = panic("failed to create list object");
            } else {
                csv->buffer_size = buffer_size;
            }
            if(status)
                list_destroy(&csv->active);
        }
        if(status)
            list_destroy(&csv->string);
    }

    return status;
}

void csv_destroy(struct csv * csv) {
    struct string * string;

    string = list_pop(&csv->string);
    while(string) {
        string_destroy(string);
        free(string);
        string = list_pop(&csv->string);
    }

    list_destroy(&csv->record);
    list_destroy(&csv->active);
    list_destroy(&csv->string);
}

int csv_parse(struct csv * csv, const char * path, csv_process_cb process, void * data) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    csvpstate * parser;
    YY_BUFFER_STATE buffer;

    if(!process) {
        status = panic("process is zero");
    } else {
        csv->process = process;
        csv->data = data;

        file = fopen(path, "r");
        if(!file) {
            status = panic("failed to open %s", path);
        } else {
            if(csvlex_init_extra(csv, &scanner)) {
                status = panic("failed to create scanner object");
            } else {
                parser = csvpstate_new();
                if(!parser) {
                    status = panic("failed to create parser object");
                } else {
                    buffer = csv_create_buffer(file, csv->buffer_size, scanner);
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

    if(csv_clear_record(csv))
        status = panic("failed to clear the record on csv object");

    return status;
}

struct string * csv_get_string(struct csv * csv) {
    int status = 0;
    struct string * string;

    string = list_pop(&csv->string);
    if(!string) {
        string = malloc(sizeof(*string));
        if(!string) {
            status = panic("failed to get pool object");
        } else {
            if(string_create(string, 64))
                status = panic("failed to create string object");
            if(status)
                free(string);
        }
    }

    if(!status) {
        if(list_push(&csv->active, string))
            status = panic("failed to push list object");
        if(status)
            if(csv_put_string(csv, string))
                status = panic("failed to put string object");
    }

    return status ? NULL : string;
}

int csv_put_string(struct csv * csv, struct string * string) {
    int status = 0;

    string_clear(string);
    if(list_push(&csv->string, string))
        status = panic("failed to push list object");

    if(status) {
        string_destroy(string);
        free(string);
    }

    return status;
}

struct string * csv_strdup_string(struct csv * csv, char * buffer, size_t length) {
    int status = 0;
    struct string * string;

    string = csv_get_string(csv);
    if(!string) {
        status = panic("failed to get string object");
    } else {
        if(string_strdup(string, buffer, length))
            status = panic("failed to strdup string object");
        if(status)
            if(csv_put_string(csv, string))
                status = panic("failed to put string object");
    }

    return status ? NULL : string;
}

int csv_push_field(struct csv * csv, struct string * string) {
    int status = 0;

    if(list_push(&csv->record, string))
        status = panic("failed to push list object");

    return status;
}

int csv_push_field_empty(struct csv * csv) {
    int status = 0;
    struct string * string;

    string = csv_get_string(csv);
    if(!string) {
        status = panic("failed to get string object");
    } else {
        if(csv_push_field(csv, string))
            status = panic("failed to push field on csv object");
        if(status)
            if(csv_put_string(csv, string))
                status = panic("failed to put string object");
    }

    return status;
}

int csv_process_record(struct csv * csv) {
    int status = 0;

    if(csv->record.root && csv->record.root != csv->record.root->next)
        if(csv->process(&csv->record, csv->data))
            status = panic("failed to process record on csv object");

    if(csv_clear_record(csv))
        status = panic("failed to clear the record on csv object");

    return status;
}

int csv_clear_record(struct csv * csv) {
    int status = 0;
    struct string * string;

    string = list_pop(&csv->active);
    while(string && !status) {
        if(csv_put_string(csv, string))
            status = panic("failed to put string on csv object");
        string = list_pop(&csv->active);
    }

    list_clear(&csv->record);

    return status;
}
