#include "script.h"

#include "script_parser.h"
#include "script_scanner.h"

int script_map_push(struct script *);
void script_map_pop(struct script *);
void script_map_clear(struct script *);

int script_logic_push(struct script *);
void script_logic_pop(struct script *);
void script_logic_clear(struct script *);

int script_stack_push(struct script *);
void script_stack_pop(struct script *);
void script_stack_clear(struct script *);

struct script_range * script_range(struct script *, char *, ...);
void script_range_clear(struct script *);

int script_parse(struct script *, char *);
int script_parse_loop(struct script *, struct string *);

int script_evaluate(struct script *, struct script_node *, int, struct script_range **);
struct script_range * script_variable(struct script *, char *);
int script_constant(struct script *, struct script_range *);

int script_undef_create(struct script_undef * undef, size_t size, struct heap * heap) {
    int status = 0;

    if(strbuf_create(&undef->strbuf, size)) {
        status = panic("failed to create strbuf object");
    } else {
        if(map_create(&undef->map, (map_compare_cb) strcmp, heap->map_pool))
            status = panic("failed to create map object");
        if(status)
            strbuf_destroy(&undef->strbuf);
    }

    return status;
}

void script_undef_destroy(struct script_undef * undef) {
    map_destroy(&undef->map);
    strbuf_destroy(&undef->strbuf);
}

int script_undef_add(struct script_undef * undef, char * identifier) {
    int status = 0;
    char * string;

    if(!map_search(&undef->map, identifier)) {
        if(strbuf_printf(&undef->strbuf, "%s", identifier)) {
            status = panic("failed to printf strbuf object");
        } else {
            string = strbuf_char(&undef->strbuf);
            if(!string) {
                status = panic("failed to char strbuf object");
            } else if(map_insert(&undef->map, string, string)) {
                status = panic("failed to insert map object");
            }
        }
    }

    return status;
}

void script_undef_print(struct script_undef * undef) {
    struct map_kv kv;

    kv = map_start(&undef->map);
    while(kv.key) {
        fprintf(stdout, "%s ", kv.key);
        kv = map_next(&undef->map);
    }
}

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
                    goto parser_fail;
                } else if(store_create(&script->store, size)) {
                    status = panic("failed to create store object");
                    goto store_fail;
                } else if(strbuf_create(&script->strbuf, size)) {
                    status = panic("failed to create strbuf object");
                    goto strbuf_fail;
                } else if(stack_create(&script->map, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto map_fail;
                } else if(stack_create(&script->logic, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto logic_fail;
                } else if(stack_create(&script->stack, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto stack_fail;
                } else if(stack_create(&script->range, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto range_fail;
                } else if(map_create(&script->function, (map_compare_cb) strcmp, heap->map_pool)) {
                    status = panic("failed to create map object");
                    goto function_fail;
                } else if(script_undef_create(&script->undef, size, heap)) {
                    status = panic("failed to create script undef object");
                    goto undef_fail;
                }
            }
        }
    }

    return status;

undef_fail:
    map_destroy(&script->function);
function_fail:
    stack_destroy(&script->range);
range_fail:
    stack_destroy(&script->stack);
stack_fail:
    stack_destroy(&script->logic);
logic_fail:
    stack_destroy(&script->map);
map_fail:
    strbuf_destroy(&script->strbuf);
strbuf_fail:
    store_destroy(&script->store);
store_fail:
    scriptpstate_delete(script->parser);
parser_fail:
    scriptlex_destroy(script->scanner);

    return status;
}
void script_destroy(struct script * script) {
    script_undef_destroy(&script->undef);
    map_destroy(&script->function);
    stack_destroy(&script->range);
    stack_destroy(&script->stack);
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
    script_stack_clear(script);
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

void script_map_clear(struct script * script) {
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

int script_stack_push(struct script * script) {
    int status = 0;
    struct stack * stack;

    stack = store_object(&script->store, sizeof(*stack));
    if(!stack) {
        status = panic("failed to object store object");
    } else if(stack_create(stack, script->heap->stack_pool)) {
        status = panic("failed to create stack object");
    } else {
        if(stack_push(&script->stack, stack))
            status = panic("failed to push stack object");
        if(status)
            stack_destroy(stack);
    }
    return status;
}

void script_stack_pop(struct script * script) {
    struct stack * stack;

    stack = stack_pop(&script->stack);
    if(stack)
        stack_destroy(stack);
}

void script_stack_clear(struct script * script) {
    struct stack * stack;

    stack = stack_pop(&script->stack);
    while(stack) {
        stack_destroy(stack);
        stack = stack_pop(&script->stack);
    }
}

struct script_range * script_range(struct script * script, char * format, ...) {
    int status = 0;
    va_list vararg;
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
            va_start(vararg, format);
            if(strbuf_vprintf(&script->strbuf, format, vararg)) {
                status = panic("failed to vprintf strbuf object");
            } else {
                range->string = strbuf_char(&script->strbuf);
                if(!range->string) {
                    status = panic("failed to string strbuf object");
                } else if(stack_push(&script->range, range)) {
                    status = panic("failed to push stack object");
                }
            }
            va_end(vararg);
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
    struct logic * logic;
    struct script_node * node;
    struct script_range * range;

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
                if(script_evaluate(script, root->root, is_logic, &range)) {
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
                if(!logic) {
                    status = panic("invalid logic");
                } else if(logic_push(logic, not, NULL)) {
                    status = panic("failed to push logic object");
                } else {
                    if(script_evaluate(script, root->root, is_logic, &range)) {
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
            if(script_evaluate(script, root, 0, &range))
                status = panic("failed to expression script object");
            break;
    }

    return status;
}

int script_evaluate(struct script * script, struct script_node * root, int flag, struct script_range ** result) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;

    struct map * map;
    struct logic * logic;
    struct stack * stack;
    struct script_range * range;
    script_function function;

    switch(root->token) {
        case script_integer:
            range = script_range(script, "%ld", root->integer);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_add(range->range, root->integer, root->integer)) {
                status = panic("failed to add range object");
            } else {
                *result = range;
            }
            break;
        case script_identifier:
            if(root->root) {
                if(script_stack_push(script)) {
                    status = panic("failed to stack push script object");
                } else {
                    stack = stack_top(&script->stack);
                    if(!stack) {
                        status = panic("invalid stack");
                    } else if(script_evaluate(script, root->root, flag | is_stack, &x)) {
                        status = panic("failed to evaluate script object");
                    } else if(!stack_top(stack) && stack_push(stack, x)) {
                        status = panic("failed to push stack object");
                    } else {
                        function = map_search(&script->function, root->identifier);
                        if(!function) {
                            if(script_undef_add(&script->undef, root->identifier)) {
                                status = panic("failed to add script undef object");
                            } else {
                                range = script_range(script, "%s", root->identifier);
                                if(!range) {
                                    status = panic("failed to range script object");
                                } else {
                                    *result = range;
                                }
                            }
                        } else if(function(script, result)) {
                            status = panic("failed to function script object");
                        }
                    }
                    script_stack_pop(script);
                }
            } else {
                range = script_variable(script, root->identifier);
                if(range) {
                    *result = range;
                } else {
                    range = script_range(script, "%s", root->identifier);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(script_constant(script, range)) {
                        status = panic("failed to constant script object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_comma:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_stack) {
                    stack = stack_top(&script->stack);
                    if(!stack) {
                        status = panic("invalid stack");
                    } else if(root->root->token == script_comma) {
                        if(stack_push(stack, y))
                            status = panic("failed to push stack object");
                    } else if(root->root->next->token == script_comma) {
                        if(stack_push(stack, x))
                            status = panic("failed to push stack object");
                    } else {
                        if(stack_push(stack, y) || stack_push(stack, x))
                            status = panic("failed to push stack object");
                    }
                }

                if(!status) {
                    range = script_range(script, "%s", y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_assign(range->range, y->range)) {
                        status = panic("failed to assign range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_assign:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_assign(range->range, y->range)) {
                    status = panic("failed to assign range object");
                } else {
                    map = stack_top(&script->map);
                    if(!map) {
                        status = panic("invalid map");
                    } else if(map_insert(map, range->string, range)) {
                        status = panic("failed to insert map object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_plus_assign:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus(range->range, x->range, y->range)) {
                    status = panic("failed to plus range object");
                } else {
                    map = stack_top(&script->map);
                    if(!map) {
                        status = panic("invalid map");
                    } else if(map_insert(map, range->string, range)) {
                        status = panic("failed to insert map object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_minus_assign:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus(range->range, x->range, y->range)) {
                    status = panic("failed to minus range object");
                } else {
                    map = stack_top(&script->map);
                    if(!map) {
                        status = panic("invalid map");
                    } else if(map_insert(map, range->string, range)) {
                        status = panic("failed to insert map object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_question:
            if(script_logic_push(script)) {
                status = panic("failed to logic push script object");
            } else {
                logic = stack_top(&script->logic);
                if(!logic) {
                    status = panic("invalid logic");
                } else if(logic_push(logic, not, NULL)) {
                    status = panic("failed to push logic object");
                } else {
                    if( script_evaluate(script, root->root, flag | is_logic, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else {
                        range = script_range(script, "%s ? %s", x->string, y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_assign(range->range, y->range)) {
                            status = panic("failed to assign range object");
                        } else {
                            *result = range;
                        }
                    }
                }
                script_logic_pop(script);
            }
            break;
        case script_colon:
            logic = stack_top(&script->logic);
            if(!logic) {
                status = panic("invalid logic");
            } else if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else if(logic_pop(logic)) {
                status = panic("failed to pop logic object");
            } else if(script_evaluate(script, root->root->next, flag, &y)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s : %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_or(range->range, x->range, y->range)) {
                    status = panic("failed to or range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_or:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s | %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_or(range->range, x->range, y->range)) {
                    status = panic("failed to bit or range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_xor:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s ^ %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_xor(range->range, x->range, y->range)) {
                    status = panic("failed to bit xor range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_and:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s & %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_and(range->range, x->range, y->range)) {
                    status = panic("failed to bit and range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_left:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s << %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_left(range->range, x->range, y->range)) {
                    status = panic("failed to bit left range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_right:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s >> %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_right(range->range, x->range, y->range)) {
                    status = panic("failed to bit right range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_plus:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s + %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus(range->range, x->range, y->range)) {
                    status = panic("failed to plus range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_minus:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s - %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus(range->range, x->range, y->range)) {
                    status = panic("failed to minus range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_multiply:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s * %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_multiply(range->range, x->range, y->range)) {
                    status = panic("failed to multiply range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_divide:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s / %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_divide(range->range, x->range, y->range)) {
                    status = panic("failed to divide range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_remainder:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s %% %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_remainder(range->range, x->range, y->range)) {
                    status = panic("failed to remainder range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_plus_unary:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "+ %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus_unary(range->range, x->range)) {
                    status = panic("failed to plus unary range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_minus_unary:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "- %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus_unary(range->range, x->range)) {
                    status = panic("failed to minus unary range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_not:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "~ %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_not(range->range, x->range)) {
                    status = panic("failed to bit not range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_increment_prefix:
        case script_increment_postfix:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s ++", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_increment(range->range, x->range)) {
                    status = panic("failed to increment range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_decrement_prefix:
        case script_decrement_postfix:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, "%s --", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_decrement(range->range, x->range)) {
                    status = panic("failed to decrement range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_or:
            if(flag & is_logic) {
                logic = stack_top(&script->logic);
                if(!logic) {
                    status = panic("invalid logic");
                } else if(logic_push(logic, or, NULL)) {
                    status = panic("failed to push logic object");
                } else {
                    if( script_evaluate(script, root->root, flag, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else if(logic_pop(logic)) {
                        status = panic("failed to pop logic object");
                    }
                }
            } else {
                if( script_evaluate(script, root->root, flag, &x) ||
                    script_evaluate(script, root->root->next, flag, &y) )
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range(script, "%s || %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_add(range->range, 0, 1)) {
                    status = panic("failed to add range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_and:
            if(flag & is_logic) {
                logic = stack_top(&script->logic);
                if(!logic) {
                    status = panic("invalid logic");
                } else if(logic_push(logic, and, NULL)) {
                    status = panic("failed to push logic object");
                } else {
                    if( script_evaluate(script, root->root, flag, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else if(logic_pop(logic)) {
                        status = panic("failed to pop logic object");
                    }
                }
            } else {
                if( script_evaluate(script, root->root, flag, &x) ||
                    script_evaluate(script, root->root->next, flag, &y) )
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range(script, "%s && %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_add(range->range, 0, 1)) {
                    status = panic("failed to add range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_not:
            if(flag & is_logic) {
                logic = stack_top(&script->logic);
                if(!logic) {
                    status = panic("invalid logic");
                } else if(logic_push(logic, not, NULL)) {
                    status = panic("failed to push logic object");
                } else {
                    if(script_evaluate(script, root->root, flag, &x)) {
                        status = panic("failed to evaluate script object");
                    } else if(logic_pop(logic)) {
                        status = panic("failed to pop logic object");
                    }
                }
            } else {
                if(script_evaluate(script, root->root, flag, &x))
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range(script, "! %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_add(range->range, 0, 1)) {
                    status = panic("failed to add range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    logic = stack_top(&script->logic);
                    if(!logic) {
                        status = panic("invalid logic");
                    } else {
                        if(root->root->token == script_identifier) {
                            range = script_range(script, "%s", x->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_equal(range->range, x->range, y->range)) {
                                status = panic("failed to equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }

                        if(root->root->next->token == script_identifier && !status) {
                            range = script_range(script, "%s", y->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_equal(range->range, y->range, x->range)) {
                                status = panic("failed to equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, "%s == %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_not_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    logic = stack_top(&script->logic);
                    if(!logic) {
                        status = panic("invalid logic");
                    } else {
                        if(root->root->token == script_identifier) {
                            range = script_range(script, "%s", x->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_not_equal(range->range, x->range, y->range)) {
                                status = panic("failed to not equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }

                        if(root->root->next->token == script_identifier && !status) {
                            range = script_range(script, "%s", y->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_not_equal(range->range, y->range, x->range)) {
                                status = panic("failed to not equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, "%s != %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_lesser:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    logic = stack_top(&script->logic);
                    if(!logic) {
                        status = panic("invalid logic");
                    } else {
                        if(root->root->token == script_identifier) {
                            range = script_range(script, "%s", x->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_lesser(range->range, x->range, y->range)) {
                                status = panic("failed to lesser range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }

                        if(root->root->next->token == script_identifier && !status) {
                            range = script_range(script, "%s", y->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_greater(range->range, y->range, x->range)) {
                                status = panic("failed to greater range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, "%s < %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_lesser_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    logic = stack_top(&script->logic);
                    if(!logic) {
                        status = panic("invalid logic");
                    } else {
                        if(root->root->token == script_identifier) {
                            range = script_range(script, "%s", x->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_lesser_equal(range->range, x->range, y->range)) {
                                status = panic("failed to lesser equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }

                        if(root->root->next->token == script_identifier && !status) {
                            range = script_range(script, "%s", y->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_greater_equal(range->range, y->range, x->range)) {
                                status = panic("failed to greater equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, "%s <= %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_greater:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    logic = stack_top(&script->logic);
                    if(!logic) {
                        status = panic("invalid logic");
                    } else {
                        if(root->root->token == script_identifier) {
                            range = script_range(script, "%s", x->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_greater(range->range, x->range, y->range)) {
                                status = panic("failed to greater range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }

                        if(root->root->next->token == script_identifier && !status) {
                            range = script_range(script, "%s", y->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_lesser(range->range, y->range, x->range)) {
                                status = panic("failed to lesser range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, "%s > %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_greater_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    logic = stack_top(&script->logic);
                    if(!logic) {
                        status = panic("invalid logic");
                    } else {
                        if(root->root->token == script_identifier) {
                            range = script_range(script, "%s", x->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_greater_equal(range->range, x->range, y->range)) {
                                status = panic("failed to greater equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }

                        if(root->root->next->token == script_identifier && !status) {
                            range = script_range(script, "%s", y->string);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else if(range_lesser_equal(range->range, y->range, x->range)) {
                                status = panic("failed to lesser equal range object");
                            } else if(logic_push(logic, cond, range)) {
                                status = panic("failed to push logic object");
                            }
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, "%s >= %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        default:
            status = panic("invalid token - %d", root->token);
            break;
    }

    return status;
}

struct script_range * script_variable(struct script * script, char * identifier) {
    struct map * map;

    map = stack_top(&script->map);

    return map ? map_search(map, identifier) : NULL;
}

int script_constant(struct script * script, struct script_range * constant) {
    int status = 0;
    struct constant_node * node;
    struct constant_range * range;

    node = constant_identifier(script->table, constant->string);
    if(node) {
        if(node->range) {
            range = node->range;
            while(range && !status) {
                if(range_add(constant->range, range->min, range->max)) {
                    status = panic("failed to add range object");
                } else {
                    range = range->next;
                }
            }
        } else if(range_add(constant->range, node->value, node->value)) {
            status = panic("failed to add range object");
        }
    }

    return status;
}
