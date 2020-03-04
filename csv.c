#include "csv.h"

#include "csv_parser.h"
#include "csv_scanner.h"

int csv_parse_loop(struct csv *, yyscan_t, csvpstate *);

int csv_create(struct csv * csv, size_t size, struct heap * heap) {
    int status = 0;

    if(strbuf_create(&csv->strbuf, size)) {
        status = panic("failed to create strbuf object");
    } else {
        csv->pool = heap_pool(heap, sizeof(struct csv_node));
        if(!csv->pool) {
            status = panic("failed to pool heap object");
        } else {
            csv->root = NULL;
            csv->last = NULL;
        }
        if(status)
            strbuf_destroy(&csv->strbuf);
    }

    return status;
}

void csv_destroy(struct csv * csv) {
    strbuf_destroy(&csv->strbuf);
}

int csv_parse(struct csv * csv, const char * path, size_t size, event_cb callback, void * context) {
    int status = 0;

    FILE * file;
    yyscan_t scanner;
    csvpstate * parser;
    YY_BUFFER_STATE buffer;

    csv->callback = callback;
    csv->context = context;

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
    int token;
    int state = YYPUSH_MORE;

    if(csv->callback(list_begin, NULL, csv->context)) {
        status = panic("failed to process list start event");
    } else {
        while(state == YYPUSH_MORE && !status) {
            token = csvlex(&value, scanner);
            if(token < 0) {
                status = panic("failed to get the next token");
            } else {
                state = csvpush_parse(parser, token, &value, csv);
                if(state && state != YYPUSH_MORE)
                    status = panic("failed to parse the current token");
            }
        }

        if(status) {
            /* skip list end on error */
        } else if(csv->callback(list_end, NULL, csv->context)) {
            status = panic("failed to process list end event");
        }
    }

    csv_reset(csv);

    return status;
}

int csv_push(struct csv * csv) {
    int status = 0;
    struct csv_node * node;

    node = pool_get(csv->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->string = strbuf_string(&csv->strbuf);
        if(!node->string) {
            status = panic("failed to string strbuf object");
        } else {
            node->next = NULL;

            if(csv->last) {
                csv->last->next = node;
            } else {
                csv->root = node;
            }
            csv->last = node;
        }
        if(status)
            pool_put(csv->pool, node);
    }

    return status;
}

int csv_pop(struct csv * csv) {
    int status = 0;
    struct csv_node * node;

    if(csv->root) {
        if(csv->callback(list_begin, NULL, csv->context)) {
            status = panic("failed to process list start event");
        } else {
            node = csv->root;
            while(node && !status) {
                if(csv->callback(scalar, node->string, csv->context))
                    status = panic("failed to process string event");
                node = node->next;
            }

            if(status) {
                /* skip list end on error */
            } else if(csv->callback(list_end, NULL, csv->context)) {
                status = panic("failed to process list end event");
            }
        }
    }

    csv_reset(csv);

    return status;
}

void csv_reset(struct csv * csv) {
    struct csv_node * node;

    csv->last = NULL;

    while(csv->root) {
        node = csv->root;
        csv->root = csv->root->next;
        pool_put(csv->pool, node);
    }

    strbuf_clear(&csv->strbuf);
}
