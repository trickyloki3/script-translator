#include "script.h"

#include "script_parser.h"
#include "script_scanner.h"

int script_map_push(struct script *);
void script_map_pop(struct script *);
int script_map_clear(struct script *);

int script_logic_push(struct script *);
void script_logic_pop(struct script *);
void script_logic_clear(struct script *);

struct script_range * script_range(struct script *);
void script_range_clear(struct script *);

char * script_string(struct script *, char *, ...);

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
                        if(strbuf_create(&script->strbuf, size)) {
                            status = panic("failed to create strbuf object");
                        } else {
                            if(stack_create(&script->map, heap->stack_pool)) {
                                status = panic("failed to create stack object");
                            } else {
                                if(stack_create(&script->logic, heap->stack_pool)) {
                                    status = panic("failed to create stack object");
                                } else {
                                    if(stack_create(&script->range, heap->stack_pool))
                                        status = panic("failed to create stack object");
                                    if(status)
                                        stack_destroy(&script->logic);
                                }
                                if(status)
                                    stack_destroy(&script->map);
                            }
                            if(status)
                                strbuf_destroy(&script->strbuf);
                        }
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
    stack_destroy(&script->range);
    stack_destroy(&script->logic);
    stack_destroy(&script->map);
    strbuf_destroy(&script->strbuf);
    store_destroy(&script->store);
    scriptpstate_delete(script->parser);
    scriptlex_destroy(script->scanner);
}

int script_compile(struct script * script, char * string) {
    int status = 0;

    if(script_parse(script, string))
        status = panic("failed to parse script object");

    script_range_clear(script);
    script_logic_clear(script);
    script_map_clear(script);
    strbuf_clear(&script->strbuf);
    store_clear(&script->store);

    return status;
}

int script_map_push(struct script * script) {
    int status = 0;
    struct map * map;
    struct map * top;

    map = store_object(&script->store, sizeof(*map));
    if(!map) {
        status = panic("failed to object store object");
    } else {
        top = stack_top(&script->map);
        if(top ? map_copy(map, top) : map_create(map, (map_compare_cb) strcmp, script->heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(stack_push(&script->map, map))
                status = panic("failed to push stack object");
            if(status)
                map_destroy(map);
        }
    }

    return status;
}

void script_map_pop(struct script * script) {
    struct map * map;

    map = stack_pop(&script->map);
    if(map)
        map_destroy(map);
}

int script_map_clear(struct script * script) {
    struct map * map;

    map = stack_pop(&script->map);
    while(map) {
        map_destroy(map);
        map = stack_pop(&script->map);
    }
}

int script_logic_push(struct script * script) {
    int status = 0;
    struct logic * logic;
    struct logic * top;

    logic = store_object(&script->store, sizeof(*logic));
    if(!logic) {
        status = panic("failed to object store object");
    } else {
        top = stack_top(&script->logic);
        if(top ? logic_copy(logic, top) : logic_create(logic, script->heap->logic_pool)) {
            status = panic("failed to create logic object");
        } else {
            if(stack_push(&script->logic, logic))
                status = panic("failed to push stack object");
            if(status)
                logic_destroy(logic);
        }
    }

    return status;
}

void script_logic_pop(struct script * script) {
    struct logic * logic;

    logic = stack_pop(&script->logic);
    if(logic)
        logic_destroy(logic);
}

void script_logic_clear(struct script * script) {
    struct logic * logic;

    logic = stack_pop(&script->logic);
    while(logic) {
        logic_destroy(logic);
        logic = stack_pop(&script->logic);
    }
}

struct script_range * script_range(struct script * script) {
    int status = 0;
    struct script_range * range;

    range = store_object(&script->store, sizeof(*range));
    if(!range) {
        status = panic("failed to object store object");
    } else {
        range->range = store_object(&script->store, sizeof(*range->range));
        if(!range->range) {
            status = panic("failed to object store object");
        }  else if(range_create(range->range, script->heap->range_pool)) {
            status = panic("failed to create range object");
        } else {
            if(stack_push(&script->range, range)) {
                status = panic("failed to push stack object");
            } else {
                range->string = NULL;
            }
            if(status)
                range_destroy(range->range);
        }
    }

    return status ? NULL : range;
}

void script_range_clear(struct script * script) {
    struct script_range * range;

    range = stack_pop(&script->range);
    while(range) {
        range_destroy(range->range);
        range = stack_pop(&script->range);
    }
}

char * script_string(struct script * script, char * format, ...) {
    int status = 0;
    va_list vararg;
    char * string;

    va_start(vararg, format);
    if(strbuf_printf(&script->strbuf, format, vararg)) {
        status = panic("failed to printf strbuf object");
    } else {
        string = strbuf_char(&script->strbuf);
        if(!string)
            status = panic("failed to char strbuf object");
    }
    va_end(vararg);

    return status ? NULL : string;
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

    yyscan_t scanner = script->scanner;
    scriptpstate * parser = script->parser;

    YY_BUFFER_STATE buffer;
    SCRIPTSTYPE value;
    int token;
    int state = YYPUSH_MORE;

    buffer = script_scan_buffer(string->string, string->length, scanner);
    if(!buffer) {
        status = panic("failed to scan buffer scanner object");
    } else {
        while(state == YYPUSH_MORE && !status) {
            token = scriptlex(&value, scanner);
            if(token < 0) {
                status = panic("failed to get the next token");
            } else {
                state = scriptpush_parse(parser, token, &value, script);
                if(state && state != YYPUSH_MORE)
                    status = panic("failed to parse the current token");
            }
        }
        scriptpop_buffer_state(scanner);
    }

    return status;
}

int script_statement(struct script * script, struct script_node * root) {
    int status = 0;
    struct script_node * node;
    struct logic * logic;

    switch(root->token) {
        case script_curly_open:
            if(script_map_push(script)) {
                status = panic("failed to map push script object");
            } else {
                node = root->root;
                while(node && !status) {
                    if(script_statement(script, node)) {
                        status = panic("failed to statement script object");
                    } else {
                        node = node->next;
                    }
                }
                script_map_pop(script);
            }
            break;
        case script_semicolon:
            /* empty statement */
            break;
        case script_for:
            /* unsupport loop */
            break;
        case script_if:
            if(script_logic_push(script)) {
                status = panic("failed to logic push script object");
            } else {
                if(!script_expression(script, root->root, is_logic)) {
                    status = panic("failed to expression script object");
                } else if(script_statement(script, root->root->next)) {
                    status = panic("failed to statement script object");
                }
                script_logic_pop(script);
            }
            break;
        case script_else:
            if(script_logic_push(script)) {
                status = panic("failed to logic push script object");
            } else {
                logic = stack_top(&script->logic);

                if(logic_push(logic, not, NULL)) {
                    status = panic("failed to push logic object");
                } else {
                    if(!script_expression(script, root->root, is_logic)) {
                        status = panic("failed to expression script object");
                    } else if(script_statement(script, root->root->next)) {
                        status = panic("failed to statement script object");
                    } else if(logic_pop(logic)) {
                        status = panic("failed to pop logic object");
                    } else if(script_statement(script, root->root->next->next)) {
                        status = panic("failed to statement script object");
                    }
                }
                script_logic_pop(script);
            }
            break;
        default:
            if(!script_expression(script, root, 0))
                status = panic("failed to expression script object");
            break;
    }

    return status;
}

struct script_range * script_expression(struct script * script, struct script_node * root, int flag) {
    int status = 0;
    struct script_range * range;

    range = script_range(script);
    if(!range)
        status = panic("failed to range push script object");

    return status ? NULL : range;
}
