#include "script.h"

#include "script_parser.h"
#include "script_scanner.h"

int script_parse(struct script *, char *);
int script_parse_loop(struct script *, struct string *);

int script_create(struct script * script, size_t size, struct heap * heap, struct table * table) {
    int status = 0;

    script->heap = heap;
    if(!script->heap) {
        status = panic("invalid heap object");
    } else {
        script->table = table;
        if(!script->table) {
            status = panic("invalid table object");
        } else {
            if(scriptlex_init_extra(&script->store, &script->scanner)) {
                status = panic("failed to create scanner object");
            } else {
                script->parser = scriptpstate_new();
                if(!script->parser) {
                    status = panic("failed to create parser object");
                } else {
                    if(store_create(&script->store, size)) {
                        status = panic("failed to create store object");
                    } else {
                        if(strbuf_create(&script->strbuf, size))
                            status = panic("failed to create strbuf object");
                        if(status)
                            store_destroy(&script->store);
                    }
                    if(status)
                        scriptpstate_delete(script->parser);
                }
                if(status)
                    scriptlex_destroy(script->scanner);
            }
        }
    }

    return status;
}
void script_destroy(struct script * script) {
    strbuf_destroy(&script->strbuf);
    store_destroy(&script->store);
    scriptpstate_delete(script->parser);
    scriptlex_destroy(script->scanner);
}

int script_compile(struct script * script, char * string) {
    int status = 0;

    if(script_parse(script, string))
        status = panic("failed to parse script object");

    strbuf_clear(&script->strbuf);
    store_clear(&script->store);

    return status;
}

int script_parse(struct script * script, char * string) {
    int status = 0;
    struct string * buffer;

    if(strbuf_printf(&script->strbuf, "%s", string)) {
        status = panic("failed to printf strbuf object");
    } else if(strbuf_putcn(&script->strbuf, '\0', 2)) {
        status = panic("failed to putcn strbuf object");
    } else {
        buffer = strbuf_string(&script->strbuf);
        if(!buffer) {
            status = panic("failed to string strbuf object");
        } else if(script_parse_loop(script, buffer)) {
            status = panic("failed to parse loop script object");
        }
    }

    return status;
}

int script_parse_loop(struct script * script, struct string * string) {
    int status = 0;

    YY_BUFFER_STATE buffer;
    SCRIPTSTYPE value;
    int token;
    int state = YYPUSH_MORE;

    buffer = script_scan_buffer(string->string, string->length, script->scanner);
    if(!buffer) {
        status = panic("failed to scan buffer scanner object");
    } else {
        while(state == YYPUSH_MORE && !status) {
            token = scriptlex(&value, script->scanner);
            if(token < 0) {
                status = panic("failed to get the next token");
            } else {
                state = scriptpush_parse(script->parser, token, &value, script);
                if(state && state != YYPUSH_MORE)
                    status = panic("failed to parse the current token");
            }
        }
        scriptpop_buffer_state(script->scanner);
    }

    return status;
}
